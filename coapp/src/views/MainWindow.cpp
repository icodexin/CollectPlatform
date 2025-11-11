#include "MainWindow.h"

#include <QGridLayout>
#include <QSplitter>
#include <QThread>

#include "BandView.h"
#include "CameraView.h"
#include "EEGView.h"
#include "SettingView.h"
#include "components/BarCard.h"
#include "components/LogBox.h"
#include "services/BandServer.h"
#include "services/CameraService.h"
#include "services/DataPipe.h"
#include "services/EEGReceiver.h"
#include "services/MqttPublisher.h"
// —— 放在所有 include 之后 ——
#ifdef ERROR
#  undef ERROR
#endif
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    initUI();
    initServices();
    initConnection();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    m_bandServer->stop();
    QMetaObject::invokeMethod(m_eegReceiver, &EEGReceiver::stop);
    m_eegThread->quit();
    m_eegThread->wait();
    event->accept();
}

void MainWindow::initUI() {
    ui_settingView = new SettingView;
    ui_settingView->setMaximumWidth(400);
    ui_eegView = new EEGView;
    ui_bandView = new BandView;
    ui_cameraView = new VideoWidget;

    auto* logView = new BarCard(tr("Log"), ":/res/icons/history.svg");
    ui_logBox = new LogBox;
    auto* logLayout = new QVBoxLayout;
    logLayout->addWidget(ui_logBox);
    logView->setContentLayout(logLayout);
    auto* logWidget = new QWidget;
    auto* logWidgetLayout = new QVBoxLayout(logWidget);
    logWidgetLayout->addWidget(logView);

    auto* devicesWidget = new QWidget;
    auto* devicesLayout = new QGridLayout(devicesWidget);
    devicesLayout->addWidget(ui_eegView, 0, 0);
    devicesLayout->addWidget(ui_bandView, 0, 1);
    devicesLayout->addWidget(ui_cameraView, 1, 0, 1, 2);

    auto* contentSplitter = new QSplitter(Qt::Vertical);
    contentSplitter->addWidget(devicesWidget);
    contentSplitter->addWidget(logWidget);
    contentSplitter->setCollapsible(0, false);

    auto* mainSplitter = new QSplitter(Qt::Horizontal);
    mainSplitter->addWidget(ui_settingView);
    mainSplitter->addWidget(contentSplitter);
    mainSplitter->setCollapsible(0, false);

    setCentralWidget(mainSplitter);
    setWindowTitle(qApp->applicationDisplayName());
}

void MainWindow::initServices() {
    m_dataPipe = new DataPipe(this);
    auto dataFetchedCbk = [this](std::unique_ptr<ISensorData> data_ptr) {
        if (m_dataPipe)
            m_dataPipe->push(std::move(data_ptr));
    };

    m_eegReceiver = new EEGReceiver(nullptr);
    m_eegReceiver->onDataFetched(dataFetchedCbk);

    m_eegThread = new QThread(this);
    m_eegReceiver->moveToThread(m_eegThread);
    connect(m_eegThread, &QThread::finished, m_eegReceiver, &QObject::deleteLater);
    m_eegThread->start();

    m_bandServer = new BandServer(this);
    m_bandServer->onDataReceived(dataFetchedCbk);

    // m_cameraService = new CameraService(this);
    // m_cameraService->updateCamera(ui_settingView->cameraDevice(), ui_settingView->cameraFormat());

    m_mqttPubService = new MqttPublishService(this);
}

void MainWindow::initConnection() {
    /********** SettingView <-> EEGReceiver <-> EEGView **********/
    connect(ui_settingView, &SettingView::requestConnectEEG, ui_eegView, &EEGView::onConnecting);
    connect(ui_settingView, &SettingView::requestDisconnectEEG, ui_eegView, &EEGView::onDisconnecting);
    connect(ui_settingView, &SettingView::requestConnectEEG, m_eegReceiver, &EEGReceiver::start);
    connect(ui_settingView, &SettingView::requestDisconnectEEG, m_eegReceiver, &EEGReceiver::stop);
    connect(m_eegReceiver, &EEGReceiver::connected, ui_settingView, &SettingView::onEEGConnected);
    connect(m_eegReceiver, &EEGReceiver::disconnected, ui_settingView, &SettingView::onEEGDisconnected);
    connect(m_eegReceiver, &EEGReceiver::connected, ui_eegView, &EEGView::onConnected);
    connect(m_eegReceiver, &EEGReceiver::disconnected, ui_eegView, &EEGView::onDisconnected);
    connect(m_eegReceiver, &EEGReceiver::dataFetched, ui_eegView, &EEGView::onDataFetched);
    connect(m_eegReceiver, &EEGReceiver::eventFetched, ui_eegView, &EEGView::onEventFetched);
    connect(m_eegReceiver, &EEGReceiver::errorOccurred, ui_eegView, &EEGView::onErrorOccurred);
    connect(m_eegReceiver, &EEGReceiver::logFetched, ui_eegView, &EEGView::log);

    /********** SettingView <-> CameraService <-> CameraView **********/
    connect(ui_settingView, &SettingView::requestOpenCamera, ui_cameraView, &VideoWidget::startVideo);
    // connect(ui_settingView, &SettingView::requestCloseCamera, m_cameraService, &CameraService::stop);
    // connect(ui_settingView, &SettingView::requestUpdateCamera, m_cameraService, &CameraService::updateCamera);
    // connect(ui_settingView, &SettingView::requestUpdateCameraFormat, m_cameraService, &CameraService::updateFormat);
    // connect(m_cameraService, &CameraService::runningChanged, ui_settingView, &SettingView::onCameraRunningChanged);
    // connect(m_cameraService, &CameraService::runningChanged, ui_cameraView,&VideoWidget::startVideo);
    // connect(m_cameraService, &CameraService::videoFrameChanged, ui_cameraView,[](){qDebug() << "videoFrameChanged";});

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
}
