#include "StreamSettingPanel.h"

#include <QtCore/QJsonObject>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>

#include "services/AuthService.h"
#include "services/CoSettingsMgr.h"
#include "services/UserApi.h"
#include "views/settings/StreamAdvancedSettingDialog.h"

namespace {
    constexpr struct {
        int width;
        int height;
    } kAllResolutions[] = {
        {1920, 1080},
        {1280, 720},
        {854, 480},
        {640, 360}
    };

    constexpr int kAllFps[] = {10, 15, 25, 30, 50, 60};

    EncoderOptions buildEncoderOptions(const FFEncoderType t) {
        const auto fillCommon = [](CommonEncodeOptions& opt) {
            opt.bitrate = static_cast<int64_t>(CoSettingsMgr::streamBitrateKbps()) * 1000;
            opt.gopSize = CoSettingsMgr::streamGopSize() * CoSettingsMgr::streamFps(); // 转换为帧数
            opt.maxBFrames = CoSettingsMgr::streamBFrames();
            opt.profile = CoSettingsMgr::streamProfile();
        };

        switch (t) {
            case FF_ET_X264: {
                X264EncodeOptions opt;
                fillCommon(opt);
                opt.preset = CoSettingsMgr::streamx264Preset();
                opt.tune = CoSettingsMgr::streamx264Tune();
                return opt;
            }
            case FF_ET_OpenH264: {
                OpenH264EncodeOptions opt;
                fillCommon(opt);
                return opt;
            }
            case FF_ET_H264_VideoToolbox: {
                VTEncodeOptions opt;
                fillCommon(opt);
                opt.realtime = CoSettingsMgr::streamVtRealtime();
                opt.allowSWFallback = CoSettingsMgr::streamVtAllowFallback();
                opt.prioritizeSpeed = CoSettingsMgr::streamVtPrioritizeSpeed();
                opt.spatialAQ = CoSettingsMgr::streamVtSpatialAQ();
                return opt;
            }
            case FF_ET_H264_QSV: {
                QSVEncodeOptions opt;
                fillCommon(opt);
                opt.preset = CoSettingsMgr::streamQsvPreset();
                opt.asyncDepth = CoSettingsMgr::streamQsvAsyncDepth();
                return opt;
            }
            case FF_ET_H264_NVENC: {
                NvEncEncodeOptions opt;
                fillCommon(opt);
                opt.preset = CoSettingsMgr::streamNvPreset();
                opt.tune = CoSettingsMgr::streamNvTune();
                opt.lookahead = CoSettingsMgr::streamNvLookahead();
                return opt;
            }
        }
        return EncoderOptions{};
    }
}

StreamSettingPanel::StreamSettingPanel(QWidget* parent)
    : QGroupBox(tr("Stream"), parent) {
    initUI();
    initConnections();
}

void StreamSettingPanel::initUI() {
    m_resolutionCombo = new QComboBox();
    m_resolutionCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    for (const auto& [width, height] : kAllResolutions) {
        const QString label = QString("%1×%2").arg(width).arg(height);
        m_resolutionCombo->addItem(label, QSize(width, height));
    }

    {
        const QSize saved = CoSettingsMgr::streamRes();
        bool found = false;
        for (int i = 0; i < m_resolutionCombo->count(); ++i) {
            if (m_resolutionCombo->itemData(i).toSize() == saved) {
                m_resolutionCombo->setCurrentIndex(i);
                found = true;
                break;
            }
        }
        if (!found)
            m_resolutionCombo->setCurrentIndex(0);
    }

    m_fpsCombo = new QComboBox();
    m_fpsCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    for (const int fps : kAllFps) {
        m_fpsCombo->addItem(QString::number(fps) + " fps", fps);
    }
    m_fpsCombo->setCurrentText(QString::number(CoSettingsMgr::streamFps()) + " fps");

    m_moreSettingsBtn = new QPushButton(tr("More Settings"));
    m_moreSettingsBtn->setFlat(true);
    m_moreSettingsBtn->setCursor(Qt::PointingHandCursor);
    m_moreSettingsBtn->setStyleSheet(
        "QPushButton { color: #2a77d4; text-decoration: underline;"
        " background: transparent; border: none; padding: 0; }"
        "QPushButton:disabled { color: #8a8a8a; }"
    );

    m_startBtn = new QPushButton(tr("Start Streaming"));
    m_pushUrlLabel = new QLabel(this);
    m_pushUrlLabel->setObjectName("settingsValueLabel");
    m_pushUrlLabel->setWordWrap(true);
    m_pushUrlLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(4);

    auto* formLayout = new QFormLayout();
    formLayout->setSpacing(4);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->addRow(tr("Resolution"), m_resolutionCombo);
    formLayout->addRow(tr("Frame Rate"), m_fpsCombo);

    auto* linkLayout = new QHBoxLayout();
    linkLayout->setContentsMargins(0, 0, 0, 0);
    linkLayout->addStretch(1);
    linkLayout->addWidget(m_moreSettingsBtn);

    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(linkLayout);
    mainLayout->addWidget(m_startBtn);
    mainLayout->addWidget(m_pushUrlLabel);

    updatePushUrlLabel();
}

void StreamSettingPanel::initConnections() {
    connect(m_startBtn, &QPushButton::clicked, this, &StreamSettingPanel::onStartBtnClicked);

    connect(m_moreSettingsBtn, &QPushButton::clicked, this, [this] {
        StreamAdvancedSettingDialog dlg(this);
        dlg.exec();
    });

    connect(m_resolutionCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
        [this](const int i) {
            const QSize sz = m_resolutionCombo->itemData(i).toSize();
            CoSettingsMgr::setStreamRes(sz);
        });
    connect(m_fpsCombo, qOverload<int>(&QComboBox::currentIndexChanged), this,
        [this](const int i) {
            const int fps = m_fpsCombo->itemData(i).toInt();
            CoSettingsMgr::setStreamFps(fps);
        });
    connect(&UserApi::instance(), &UserApi::currentUserFetched, this, [this](const QJsonObject&) {
        updatePushUrlLabel();
    });
    connect(&AuthService::instance(), &AuthService::authenticationChanged, this, [this](const bool) {
        updatePushUrlLabel();
    });
}

PushConfig StreamSettingPanel::buildConfig() const {
    PushConfig cfg;
    cfg.rtspUrl = CoSettingsMgr::streamUrl();
    cfg.fps = m_fpsCombo->currentData().toInt();

    const QSize sz = m_resolutionCombo->currentData().toSize();
    cfg.setResolution(sz);

    cfg.encoderType = CoSettingsMgr::streamEncoderType();

    cfg.encoderOptions = buildEncoderOptions(cfg.encoderType);
    return cfg;
}

void StreamSettingPanel::onStartBtnClicked() {
    if (m_startBtn->text() == tr("Start Streaming")) {
        if (CoSettingsMgr::authUnifiedId().trimmed().isEmpty()) {
            QMessageBox::warning(this, tr("Warning"),
                tr("The unified identity ID is not ready yet. Please wait a moment and try again."));
            return;
        }
        updatePushUrlLabel();
        m_startBtn->setText(tr("Starting..."));
        m_startBtn->setEnabled(false);
        emit requestStart(buildConfig());
    }
    else if (m_startBtn->text() == tr("Stop Streaming")) {
        m_startBtn->setText(tr("Stopping..."));
        m_startBtn->setEnabled(false);
        emit requestStop();
    }
}

void StreamSettingPanel::handleStarted() const {
    m_startBtn->setText(tr("Stop Streaming"));
    m_startBtn->setEnabled(true);
}

void StreamSettingPanel::handleStopped() const {
    m_startBtn->setText(tr("Start Streaming"));
    m_startBtn->setEnabled(true);
    m_resolutionCombo->setEnabled(true);
    m_fpsCombo->setEnabled(true);
    m_moreSettingsBtn->setEnabled(true);
}

void StreamSettingPanel::handleStateChanged(const PushWorkerState state) const {
    switch (state) {
        case PushWorkerState::Idle:
            handleStopped();
            break;
        case PushWorkerState::Starting:
            m_startBtn->setText(tr("Starting..."));
            m_startBtn->setEnabled(false);
            m_resolutionCombo->setEnabled(false);
            m_fpsCombo->setEnabled(false);
            m_moreSettingsBtn->setEnabled(false);
            break;
        case PushWorkerState::Streaming:
            handleStarted();
            break;
        case PushWorkerState::Stopping:
            m_startBtn->setText(tr("Stopping..."));
            m_startBtn->setEnabled(false);
            break;
        case PushWorkerState::Error:
            m_startBtn->setText(tr("Start Streaming"));
            m_startBtn->setEnabled(true);
            handleStopped();
            break;
    }
}

void StreamSettingPanel::updatePushUrlLabel() {
    if (!m_pushUrlLabel)
        return;

    m_pushUrlLabel->setText(tr("Push URL: %1").arg(CoSettingsMgr::streamUrl()));
}
