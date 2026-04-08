#include "StreamAdvancedSettingDialog.h"

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>

#include "services/CoSettingsMgr.h"

namespace {
    constexpr int encoderType2Page(const FFEncoderType type) {
        switch (type) {
            case FF_ET_X264: return 0;
            case FF_ET_OpenH264: return 1;
            case FF_ET_H264_VideoToolbox: return 2;
            case FF_ET_H264_QSV: return 3;
            case FF_ET_H264_NVENC: return 4;
        }
        return 0;
    }
} // namespace

StreamAdvancedSettingDialog::StreamAdvancedSettingDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Stream Advanced Settings"));
    setModal(true);
    initUI();
    initConnections();
}

void StreamAdvancedSettingDialog::initUI() {
    m_bitrateSpinBox = new QSpinBox();
    m_bitrateSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_bitrateSpinBox->setRange(100, 50000);
    m_bitrateSpinBox->setValue(CoSettingsMgr::streamBitrateKbps());
    m_bitrateSpinBox->setSuffix(" kbps");
    m_bitrateSpinBox->setSingleStep(100);

    m_profileCombo = new QComboBox();
    m_profileCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    for (const auto& [name, profile] : FFMpegHelper::getProfileOptions()) {
        m_profileCombo->addItem(name, profile);
    }
    m_profileCombo->setCurrentText(FFMpegHelper::getProfileName(CoSettingsMgr::streamProfile()));

    m_gopSpinBox = new QSpinBox();
    m_gopSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_gopSpinBox->setRange(0, 20);
    m_gopSpinBox->setSuffix(" s");
    m_gopSpinBox->setSpecialValueText(tr("0s (Auto)"));
    m_gopSpinBox->setValue(CoSettingsMgr::streamGopSize());

    m_bFramesSpinBox = new QSpinBox();
    m_bFramesSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_bFramesSpinBox->setRange(0, 5);
    m_bFramesSpinBox->setSpecialValueText(tr("0 (Disabled)"));
    m_bFramesSpinBox->setValue(CoSettingsMgr::streamBFrames());

    m_encoderCombo = new QComboBox();
    m_encoderCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_encoderCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_encoderCombo->setMinimumContentsLength(12);
    populateEncoders();

    // libx264 页
    auto* x264Page = new QWidget();
    auto* x264Layout = new QFormLayout(x264Page);
    x264Layout->setSpacing(4);
    x264Layout->setContentsMargins(0, 4, 0, 0);
    x264Layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_x264PresetCombo = new QComboBox();
    m_x264PresetCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_x264PresetCombo->addItems({
        "ultrafast", "superfast", "veryfast", "faster",
        "fast", "medium", "slow", "veryslow"
    });
    m_x264PresetCombo->setCurrentText(CoSettingsMgr::streamx264Preset());

    m_x264TuneCombo = new QComboBox();
    m_x264TuneCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_x264TuneCombo->addItems({
        "zerolatency", "film", "animation", "grain",
        "stillimage", "psnr", "ssim", "fastdecode"
    });
    m_x264TuneCombo->setCurrentText(CoSettingsMgr::streamx264Tune());

    x264Layout->addRow(tr("Preset"), m_x264PresetCombo);
    x264Layout->addRow(tr("Tune"), m_x264TuneCombo);

    // openH264 页
    auto* o264Page = new QWidget();
    auto* o264Layout = new QVBoxLayout(o264Page);
    o264Layout->setSpacing(4);
    o264Layout->setContentsMargins(0, 4, 0, 0);

    m_o264InfoLabel = new QLabel(tr(
        "No Options.\n\nOpenH264 is a software encoder provided by Cisco. It has a smaller feature set and lower performance compared to libx264. However, it is royalty-free and may be used in commercial applications without licensing fees."));
    m_o264InfoLabel->setWordWrap(true);

    o264Layout->addWidget(m_o264InfoLabel);
    o264Layout->addStretch(1);

    // VideoToolbox 页
    auto* vtPage = new QWidget();
    auto* vtLayout = new QFormLayout(vtPage);
    vtLayout->setSpacing(4);
    vtLayout->setContentsMargins(0, 4, 0, 0);
    vtLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_vtRealtimeCheck = new QCheckBox(tr("Realtime Encoding"));
    m_vtFallbackCheck = new QCheckBox(tr("Allow SW Fallback"));
    m_vtPriorSpeedCheck = new QCheckBox(tr("Prioritize Speed"));
    m_vtSpatialAQCheck = new QCheckBox(tr("Spatial Adaptive Quantization"));
    m_vtRealtimeCheck->setChecked(CoSettingsMgr::streamVtRealtime());
    m_vtFallbackCheck->setChecked(CoSettingsMgr::streamVtAllowFallback());
    m_vtPriorSpeedCheck->setChecked(CoSettingsMgr::streamVtPrioritizeSpeed());
    m_vtSpatialAQCheck->setChecked(CoSettingsMgr::streamVtSpatialAQ());

    vtLayout->addRow(m_vtRealtimeCheck);
    vtLayout->addRow(m_vtFallbackCheck);
    vtLayout->addRow(m_vtPriorSpeedCheck);
    vtLayout->addRow(m_vtSpatialAQCheck);

    // QSV 页
    auto* qsvPage = new QWidget();
    auto* qsvLayout = new QFormLayout(qsvPage);
    qsvLayout->setSpacing(4);
    qsvLayout->setContentsMargins(0, 4, 0, 0);
    qsvLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_qsvPresetCombo = new QComboBox();
    m_qsvPresetCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_qsvPresetCombo->addItems({"veryfast", "faster", "fast", "medium"});
    m_qsvPresetCombo->setCurrentText(CoSettingsMgr::streamQsvPreset());

    m_qsvAsyncDepthSpinBox = new QSpinBox();
    m_qsvAsyncDepthSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_qsvAsyncDepthSpinBox->setAlignment(Qt::AlignCenter);
    m_qsvAsyncDepthSpinBox->setRange(1, 16);
    m_qsvAsyncDepthSpinBox->setValue(CoSettingsMgr::streamQsvAsyncDepth());

    qsvLayout->addRow(tr("Preset"), m_qsvPresetCombo);
    qsvLayout->addRow(tr("Async Depth"), m_qsvAsyncDepthSpinBox);

    // NVENC 页
    auto* nvPage = new QWidget();
    auto* nvLayout = new QFormLayout(nvPage);
    nvLayout->setSpacing(4);
    nvLayout->setContentsMargins(0, 4, 0, 0);
    nvLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_nvPresetCombo = new QComboBox();
    m_nvPresetCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_nvPresetCombo->addItems({"p1", "p2", "p3", "p4", "p5", "p6", "p7"});
    m_nvPresetCombo->setCurrentText(CoSettingsMgr::streamNvPreset());

    m_nvTuneCombo = new QComboBox();
    m_nvTuneCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_nvTuneCombo->addItems({"ull", "ll", "hq", "lossless"});
    m_nvTuneCombo->setCurrentText(CoSettingsMgr::streamNvTune());

    m_nvLookaheadSpinBox = new QSpinBox();
    m_nvLookaheadSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_nvLookaheadSpinBox->setAlignment(Qt::AlignCenter);
    m_nvLookaheadSpinBox->setRange(0, 32);
    m_nvLookaheadSpinBox->setValue(CoSettingsMgr::streamNvLookahead());
    m_nvLookaheadSpinBox->setSpecialValueText(tr("0 (Disabled)"));

    nvLayout->addRow(tr("Preset"), m_nvPresetCombo);
    nvLayout->addRow(tr("Tune"), m_nvTuneCombo);
    nvLayout->addRow(tr("Lookahead"), m_nvLookaheadSpinBox);

    // 组装 stacked widget
    m_optStack = new QStackedWidget();
    m_optStack->addWidget(x264Page);
    m_optStack->addWidget(o264Page);
    m_optStack->addWidget(vtPage);
    m_optStack->addWidget(qsvPage);
    m_optStack->addWidget(nvPage);

    const int savedEncoderIdx = m_encoderCombo->currentIndex();
    if (savedEncoderIdx >= 0 && savedEncoderIdx < m_encoders.size()) {
        m_optStack->setCurrentIndex(encoderType2Page(m_encoders[savedEncoderIdx].type));
    }

    auto* encoderSpecificBox = new QGroupBox(tr("Encoder Options"));
    auto* encoderSpecificLayout = new QVBoxLayout(encoderSpecificBox);
    encoderSpecificLayout->setContentsMargins(6, 6, 6, 6);
    encoderSpecificLayout->setSpacing(2);
    encoderSpecificLayout->addWidget(m_optStack);

    auto* formWidget = new QWidget();
    auto* formLayout = new QFormLayout(formWidget);
    formLayout->setSpacing(4);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    formLayout->addRow(tr("Bitrate"), m_bitrateSpinBox);
    formLayout->addRow(tr("Profile"), m_profileCombo);
    formLayout->addRow(tr("GOP Size"), m_gopSpinBox);
    formLayout->addRow(tr("B-Frames"), m_bFramesSpinBox);
    formLayout->addRow(tr("Encoder"), m_encoderCombo);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(6);
    mainLayout->addWidget(formWidget);
    mainLayout->addWidget(encoderSpecificBox);
    mainLayout->addWidget(m_buttonBox);
}

void StreamAdvancedSettingDialog::initConnections() {
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    connect(m_encoderCombo, qOverload<int>(&QComboBox::currentIndexChanged),
        this, &StreamAdvancedSettingDialog::onEncoderIndexChanged);

    connect(m_bitrateSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &CoSettingsMgr::setStreamBitrateKbps);
    connect(m_profileCombo, &QComboBox::currentTextChanged, this, [this] {
        CoSettingsMgr::setStreamProfile(m_profileCombo->currentData().value<FFProfile>());
    });
    connect(m_gopSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &CoSettingsMgr::setStreamGopSize);
    connect(m_bFramesSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &CoSettingsMgr::setStreamBFrames);

    connect(m_x264PresetCombo, &QComboBox::currentTextChanged, this, &CoSettingsMgr::setStreamx264Preset);
    connect(m_x264TuneCombo, &QComboBox::currentTextChanged, this, &CoSettingsMgr::setStreamx264Tune);

    connect(m_vtRealtimeCheck, &QCheckBox::toggled, this, &CoSettingsMgr::setStreamVtRealtime);
    connect(m_vtFallbackCheck, &QCheckBox::toggled, this, &CoSettingsMgr::setStreamVtAllowFallback);
    connect(m_vtPriorSpeedCheck, &QCheckBox::toggled, this, &CoSettingsMgr::setStreamVtPrioritizeSpeed);
    connect(m_vtSpatialAQCheck, &QCheckBox::toggled, this, &CoSettingsMgr::setStreamVtSpatialAQ);

    connect(m_qsvPresetCombo, &QComboBox::currentTextChanged, this, &CoSettingsMgr::setStreamQsvPreset);
    connect(m_qsvAsyncDepthSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &CoSettingsMgr::setStreamQsvAsyncDepth);

    connect(m_nvPresetCombo, &QComboBox::currentTextChanged, this, &CoSettingsMgr::setStreamNvPreset);
    connect(m_nvTuneCombo, &QComboBox::currentTextChanged, this, &CoSettingsMgr::setStreamNvTune);
    connect(m_nvLookaheadSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, &CoSettingsMgr::setStreamNvLookahead);
}

void StreamAdvancedSettingDialog::populateEncoders() {
    m_encoders = FFMpegHelper::getAvailableEncoders();

    const FFEncoderType savedType = CoSettingsMgr::streamEncoderType();

    for (int i = 0; i < m_encoders.size(); ++i) {
        const FFEncoderInfo& info = m_encoders[i];
        m_encoderCombo->addItem(info.display, info.type);
    }

    m_encoderCombo->setEnabled(!m_encoders.isEmpty());
    m_encoderCombo->setCurrentText(FFMpegHelper::getCodecDisplay(savedType));
}

void StreamAdvancedSettingDialog::onEncoderIndexChanged(const int index) {
    if (index < 0 || index >= m_encoders.size())
        return;
    m_optStack->setCurrentIndex(encoderType2Page(m_encoders[index].type));
    CoSettingsMgr::setStreamEncoderType(m_encoders[index].type);
}
