#include "MainWindow.h"

#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStyle>

#include "BandView.h"
#include "CameraView.h"
#include "EEGView.h"
#include "SettingsDialog.h"
#include "SettingView.h"
#include "UserInfoDialog.h"
#include "components/BarCard.h"
#include "components/LogBox.h"
#include "network/HttpMgr.h"
#include "services/BandServer.h"
#include "services/CameraService.h"
#include "services/DataPipe.h"
#include "services/EEGRecvService.h"
#include "services/MqttPublisher.h"
#include "services/UserApi.h"
#include "services/VideoPushService.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    initUI();
    initServices();
    initConnection();
}

void MainWindow::setQuitOnClose(const bool enabled) {
    m_quitOnClose = enabled;
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_settingsDialog)
        m_settingsDialog->close();
    if (m_userDialog)
        m_userDialog->close();

    m_bandServer->stop();
    m_eegRecvService->stop();
    event->accept();
    emit windowClosed(m_quitOnClose);
}

void MainWindow::initUI() {
    ui_settingView = new SettingView;
    ui_settingView->setMaximumWidth(400);
    ui_eegView = new EEGView;
    ui_bandView = new BandView;
    ui_cameraView = new CameraView;
    ui_logBox = new LogBox;

    // 日志视图
    auto* logView = new BarCard(tr("Log"), ":/res/icons/history.svg");
    {
        ui_logBox->setStyleSheet("QPlainTextEdit { background-color: rgba(255, 255, 255, 0);}");
        auto* logLayout = new QVBoxLayout;
        logLayout->addWidget(ui_logBox);
        logLayout->setContentsMargins(4, 4, 4, 4);
        logView->setContentsMargins(8, 8, 8, 8);
        logView->setContentLayout(logLayout);
        logView->setMaximumHeight(150);
    }

    // 设备视图
    auto* devicesView = new QWidget;
    {
        auto* devicesLayout = new QGridLayout(devicesView);
        devicesLayout->setContentsMargins(8, 8, 8, 8);
        devicesLayout->setSpacing(8);
        devicesLayout->addWidget(ui_eegView, 0, 0);
        devicesLayout->addWidget(ui_bandView, 0, 1);
        devicesLayout->addWidget(ui_cameraView, 1, 0, 1, 2);
    }

    // 内容区分割器
    auto* contentSplitter = new QSplitter(Qt::Vertical);
    {
        contentSplitter->addWidget(devicesView);
        contentSplitter->addWidget(logView);
        contentSplitter->setCollapsible(0, false);
        contentSplitter->setCollapsible(1, false);
    }

    // 控制栏按钮
    ui_sidebarToggleButton = new QPushButton;
    auto* settingsButton = new QPushButton;
    auto* userButton = new QPushButton;
    {
        ui_sidebarToggleButton->setObjectName("icon");
        ui_sidebarToggleButton->setIcon(QIcon(":/res/icons/sidebar.svg"));
        ui_sidebarToggleButton->setIconSize(QSize(18, 18));

        settingsButton->setObjectName("icon");
        settingsButton->setIcon(QIcon(":/res/icons/setting.svg"));
        settingsButton->setIconSize(QSize(18, 18));

        userButton->setObjectName("icon");
        userButton->setIcon(QIcon(":/res/icons/user.svg"));
        userButton->setIconSize(QSize(18, 18));
    }

    // 顶部控制栏
    auto* topControlBar = new QWidget;
    {
        topControlBar->setObjectName("topControlBar");
        auto* topControlLayout = new QHBoxLayout(topControlBar);
        topControlLayout->setContentsMargins(8, 4, 8, 4);
        topControlLayout->setSpacing(2);

        ui_networkStatusLabel = new QLabel(tr("Service Unreachable"), topControlBar);
        ui_networkStatusLabel->setObjectName("networkStatusLabel");
        ui_networkStatusLabel->hide();

        topControlLayout->addWidget(ui_sidebarToggleButton, 0, Qt::AlignLeft);
        topControlLayout->addSpacing(8);
        topControlLayout->addWidget(ui_networkStatusLabel, 0, Qt::AlignLeft);
        topControlLayout->addStretch(1);
        topControlLayout->addWidget(settingsButton, 0, Qt::AlignRight);
        topControlLayout->addWidget(userButton, 0, Qt::AlignRight);
    }

    // 内容区
    auto* contentWidget = new QWidget;
    {
        auto* contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->setSpacing(0);
        contentLayout->addWidget(topControlBar);
        contentLayout->addWidget(contentSplitter, 1);
    }

    // 主分割器
    ui_mainSplitter = new QSplitter(Qt::Horizontal);
    {
        ui_mainSplitter->addWidget(ui_settingView);
        ui_mainSplitter->addWidget(contentWidget);
        ui_mainSplitter->setCollapsible(0, false);
        ui_mainSplitter->setCollapsible(1, false);
        ui_mainSplitter->setStretchFactor(0, 0);
        ui_mainSplitter->setStretchFactor(1, 1);
        ui_mainSplitter->setSizes({m_lastSidebarWidth, 960});
    }

    connect(ui_sidebarToggleButton, &QPushButton::clicked, this, &MainWindow::toggleSidebar);
    connect(settingsButton, &QPushButton::clicked, this, &MainWindow::showSettingsDialog);
    connect(userButton, &QPushButton::clicked, this, &MainWindow::showUserInfoDialog);

    setCentralWidget(ui_mainSplitter);
    setWindowTitle(qApp->applicationDisplayName());
}

void MainWindow::initServices() {
    m_dataPipe = new DataPipe(this);
    auto dataFetchedCbk = [this](std::unique_ptr<ISensorData> data_ptr) {
        if (m_dataPipe)
            m_dataPipe->push(std::move(data_ptr));
    };

    m_eegRecvService = new EEGRecvService(this);
    m_eegRecvService->onDataFetched(dataFetchedCbk);

    m_bandServer = new BandServer(this);
    m_bandServer->onDataReceived(dataFetchedCbk);

    m_cameraService = new CameraService(this);
    m_cameraService->updateCamera(ui_settingView->cameraDevice(), ui_settingView->cameraFormat());

    m_mqttPubService = new MqttPublishService(this);

    m_videoPushService = new VideoPushService(this);
}

void MainWindow::initConnection() {
    /********** SettingView <-> EEGRecvService <-> EEGView **********/
    connect(ui_settingView, &SettingView::requestConnectEEG, ui_eegView, &EEGView::onConnecting);
    connect(ui_settingView, &SettingView::requestDisconnectEEG, ui_eegView, &EEGView::onDisconnecting);
    connect(ui_settingView, &SettingView::requestConnectEEG, m_eegRecvService, &EEGRecvService::start);
    connect(ui_settingView, &SettingView::requestDisconnectEEG, m_eegRecvService, &EEGRecvService::stop);
    connect(m_eegRecvService, &EEGRecvService::connected, ui_settingView, &SettingView::onEEGConnected);
    connect(m_eegRecvService, &EEGRecvService::disconnected, ui_settingView, &SettingView::onEEGDisconnected);
    connect(m_eegRecvService, &EEGRecvService::connected, ui_eegView, &EEGView::onConnected);
    connect(m_eegRecvService, &EEGRecvService::disconnected, ui_eegView, &EEGView::onDisconnected);
    connect(m_eegRecvService, &EEGRecvService::dataFetched, ui_eegView, &EEGView::onDataFetched);
    connect(m_eegRecvService, &EEGRecvService::eventFetched, ui_eegView, &EEGView::onEventFetched);
    connect(m_eegRecvService, &EEGRecvService::errorOccurred, ui_eegView, &EEGView::onErrorOccurred);
    connect(m_eegRecvService, &EEGRecvService::logFetched, ui_eegView, &EEGView::log);

    /********** SettingView <-> CameraService <-> CameraView **********/
    connect(ui_settingView, &SettingView::requestOpenCamera, m_cameraService, &CameraService::start);
    connect(ui_settingView, &SettingView::requestCloseCamera, m_cameraService, &CameraService::stop);
    connect(ui_settingView, &SettingView::requestUpdateCamera, m_cameraService, &CameraService::updateCamera);
    connect(ui_settingView, &SettingView::requestUpdateCameraFormat, m_cameraService, &CameraService::updateFormat);
    connect(m_cameraService, &CameraService::runningChanged, ui_settingView, &SettingView::onCameraRunningChanged);
    connect(m_cameraService, &CameraService::runningChanged, ui_cameraView, &CameraView::setPlaying);
    connect(m_cameraService, &CameraService::videoFrameChanged, ui_cameraView, &CameraView::setFrame);

    /********** SettingView <-> BandServer <-> BandView **********/
    connect(ui_settingView, &SettingView::requestStartBandService, ui_bandView, &BandView::onConnecting);
    connect(ui_settingView, &SettingView::requestStopBandService, ui_bandView, &BandView::onDisconnecting);
    connect(ui_settingView, &SettingView::requestStartBandService, m_bandServer, &BandServer::start);
    connect(ui_settingView, &SettingView::requestStopBandService, m_bandServer, &BandServer::stop);
    connect(m_bandServer, &BandServer::runningChanged, ui_settingView, &SettingView::onBandServiceRunningChanged);
    connect(m_bandServer, &BandServer::runningChanged, ui_bandView, &BandView::onConnectionStatusChanged);
    connect(m_bandServer, &BandServer::errorOccurred, ui_bandView, &BandView::onErrorOccurred);
    connect(m_bandServer, &BandServer::clientConnected, ui_bandView, &BandView::onClientConnected);
    connect(m_bandServer, &BandServer::clientDisconnected, ui_bandView, &BandView::onClientDisconnected);
    connect(m_bandServer, &BandServer::dataReceived, ui_bandView, &BandView::onDataReceived);

    /********** MQTT Publish Service **********/
    connect(ui_settingView, &SettingView::requestStartMqtt, m_mqttPubService,
        qOverload<const QString&, quint16, const QString&, const QString&, const QString&>(&MqttPublishService::start));
    connect(ui_settingView, &SettingView::requestStopMqtt, m_mqttPubService, &MqttPublishService::stop);
    connect(m_mqttPubService, &MqttPublishService::started, ui_settingView, &SettingView::onMqttConnected);
    connect(m_mqttPubService, &MqttPublishService::stopped, ui_settingView, &SettingView::onMqttDisconnected);
    connect(m_mqttPubService, &MqttPublishService::started, this, [=] {
        m_dataPipe->setStudentId(ui_settingView->mqttUserId());
        m_dataPipe->allowPush(true);
        ui_logBox->log(LogMessage::SUCCESS, tr("MQTT connected"));
    });
    connect(m_mqttPubService, &MqttPublishService::stopped, this, [=] {
        m_dataPipe->allowPush(false);
        ui_logBox->log(LogMessage::WARN, tr("MQTT disconnected"));
    });
    connect(m_mqttPubService, &MqttPublishService::errorOccurred,
        this, [=](QMqttClient::ClientError error, const QString& msg) {
            ui_logBox->log(LogMessage::ERROR, tr("MQTT error: %1").arg(msg));
        });
    connect(m_mqttPubService, &MqttPublishService::messageSent, this, [=](const qint32 msgId) {
        ui_logBox->log(LogMessage::INFO, tr("MQTT message sent, ID: %1").arg(msgId));
    });

    connect(m_dataPipe, &DataPipe::dataReady, m_mqttPubService,
        qOverload<const QString&, const QByteArray&>(&MqttPublishService::publish)
    );

    /********** SettingView <-> VideoPushService **********/
    connect(ui_settingView, &SettingView::requestStartVideoPush, m_videoPushService, &VideoPushService::start);
    connect(ui_settingView, &SettingView::requestStopVideoPush, m_videoPushService, &VideoPushService::stop);
    connect(m_videoPushService, &VideoPushService::stateChanged, ui_settingView, &SettingView::onVideoPushStateChanged);
    connect(m_cameraService, &CameraService::videoFrameChanged, m_videoPushService, &VideoPushService::pushFrame);
    connect(m_videoPushService, &VideoPushService::stateChanged, this, [=](PushWorkerState state) {
        switch (state) {
            case PushWorkerState::Starting:
                ui_logBox->log(LogMessage::INFO, tr("Video push starting..."));
                break;
            case PushWorkerState::Streaming:
                ui_logBox->log(LogMessage::SUCCESS, tr("Video push started"));
                break;
            case PushWorkerState::Stopping:
                ui_logBox->log(LogMessage::INFO, tr("Video push stopping..."));
                break;
            case PushWorkerState::Idle:
                ui_logBox->log(LogMessage::WARN, tr("Video push stopped"));
                break;
            case PushWorkerState::Error:
                ui_logBox->log(LogMessage::ERROR, tr("Video push error"));
                break;
        }
    });
    connect(m_videoPushService, &VideoPushService::errorOccurred, this, [=](int /*code*/, const QString& msg) {
        ui_logBox->log(LogMessage::ERROR, tr("Video push error: %1").arg(msg));
    });
    connect(m_videoPushService, &VideoPushService::statsUpdated, this, [=](const PushStats& st) {
        ui_cameraView->setStreamStats(st.currentFps, st.currentBitrateKbps);
    });
    connect(m_videoPushService, &VideoPushService::stateChanged, this, [=](PushWorkerState state) {
        if (state == PushWorkerState::Idle || state == PushWorkerState::Error) {
            ui_cameraView->clearStreamStats();
        }
    });

    connect(&HttpMgr::instance(), &HttpMgr::requestFinished, this,
        [this](const QString&, const QUrl&, const int) {
            updateNetworkStatus(false, {});
        });
    connect(&HttpMgr::instance(), &HttpMgr::requestFailed, this,
        [this](const QString&, const QUrl&, const int statusCode, const QString& errorString) {
            if (statusCode == 0 || statusCode == 502) {
                updateNetworkStatus(true, tr("Service Unreachable"));
                ui_logBox->log(LogMessage::WARN, tr("Network error: %1").arg(errorString));
                return;
            }

            updateNetworkStatus(false, {});
        });
    connect(&HttpMgr::instance(), &HttpMgr::sslErrorsOccurred, this,
        [this](const QUrl&, const QStringList&) {
            updateNetworkStatus(false, {});
        });
    connect(&UserApi::instance(), &UserApi::currentUserSessionInvalid, this, [this] {
        ui_logBox->log(LogMessage::WARN, tr("Current user is no longer valid. Session has been cleared."));
    });
}

void MainWindow::toggleSidebar() {
    if (!ui_mainSplitter)
        return;

    if (!m_sidebarCollapsed) {
        const auto sizes = ui_mainSplitter->sizes();
        if (!sizes.isEmpty() && sizes.at(0) > 0)
            m_lastSidebarWidth = sizes.at(0);

        ui_settingView->hide();
        ui_mainSplitter->setSizes({0, qMax(1, width())});
        m_sidebarCollapsed = true;
    } else {
        ui_settingView->show();
        const int sidebarWidth = qMax(240, m_lastSidebarWidth);
        ui_mainSplitter->setSizes({sidebarWidth, qMax(1, width() - sidebarWidth)});
        m_sidebarCollapsed = false;
    }
}

void MainWindow::updateNetworkStatus(const bool visible, const QString& text) {
    if (!ui_networkStatusLabel)
        return;

    ui_networkStatusLabel->setVisible(visible);
    if (!text.isEmpty())
        ui_networkStatusLabel->setText(text);
}

void MainWindow::showSettingsDialog() {
    if (!m_settingsDialog) {
        m_settingsDialog = new SettingsDialog(this);
    }

    m_settingsDialog->show();
    m_settingsDialog->raise();
    m_settingsDialog->activateWindow();
}

void MainWindow::showUserInfoDialog() {
    if (!m_userDialog) {
        m_userDialog = new UserInfoDialog(this);
    }

    m_userDialog->show();
    m_userDialog->raise();
    m_userDialog->activateWindow();
}
