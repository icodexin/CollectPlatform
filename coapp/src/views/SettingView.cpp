#include "SettingView.h"

#include <QVBoxLayout>

#include "settings/BandSettingPanel.h"
#include "settings/CameraSettingPanel.h"
#include "settings/EEGSettingPanel.h"
#include "settings/InfoPanel.h"
#include "settings/MqttSettingPanel.h"

SettingView::SettingView(QWidget* parent)
    : QWidget(parent) {
    initUI();
    initConnection();
}

QString SettingView::eegAddress() const {
    return ui_eegPanel->address();
}

int SettingView::eegPort() const {
    return ui_eegPanel->port();
}

int SettingView::bandServicePort() const {
    return ui_bandPanel->port();
}

QCameraDevice SettingView::cameraDevice() const {
    return ui_cameraPanel->device();
}

QCameraFormat SettingView::cameraFormat() const {
    return ui_cameraPanel->format();
}

void SettingView::onEEGConnected() const {
    ui_eegPanel->handleConnected();
}

void SettingView::onEEGDisconnected() const {
    ui_eegPanel->handleDisconnected();
}

void SettingView::onEEGReceiverRunningChanged(const bool connected) const {
    if (connected) {
        onEEGConnected();
    } else {
        onEEGDisconnected();
    }
}

void SettingView::onBandServiceStarted() const {
    ui_bandPanel->handleServiceStarted();
}

void SettingView::onBandServiceStopped() const {
    ui_bandPanel->handleServiceStopped();
}

void SettingView::onBandServiceRunningChanged(const bool running) const {
    ui_bandPanel->handleServiceRunningChanged(running);
}

void SettingView::onCameraOpened() const {
    ui_cameraPanel->handleOpened();
}

void SettingView::onCameraClosed() const {
    ui_cameraPanel->handleClosed();
}

void SettingView::onCameraRunningChanged(const bool running) const {
    ui_cameraPanel->handleRunningChanged(running);
}

void SettingView::onMqttConnected() const {
    ui_mqttPanel->handleConnected();
}

void SettingView::onMqttDisconnected() const {
    ui_mqttPanel->handleDisconnected();
}

void SettingView::initUI() {
    ui_eegPanel = new EEGSettingPanel(this);
    ui_bandPanel = new BandSettingPanel(this);
    ui_cameraPanel = new CameraSettingPanel(this);
    ui_mqttPanel = new MqttSettingPanel(this);
    ui_infoPanel = new InfoPanel(this);

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->addWidget(ui_eegPanel);
    layout->addWidget(ui_bandPanel);
    layout->addWidget(ui_cameraPanel);
    layout->addWidget(ui_mqttPanel);
    layout->addStretch(1);
    layout->addWidget(ui_infoPanel);
}

void SettingView::initConnection() {
    connect(ui_eegPanel, &EEGSettingPanel::requestConnect, this, &SettingView::requestConnectEEG);
    connect(ui_eegPanel, &EEGSettingPanel::requestDisconnect, this, &SettingView::requestDisconnectEEG);

    connect(ui_bandPanel, &BandSettingPanel::requestStartService, this, &SettingView::requestStartBandService);
    connect(ui_bandPanel, &BandSettingPanel::requestStopService, this, &SettingView::requestStopBandService);

    connect(ui_cameraPanel, &CameraSettingPanel::requestOpen, this, &SettingView::requestOpenCamera);
    connect(ui_cameraPanel, &CameraSettingPanel::requestClose, this, &SettingView::requestCloseCamera);
    connect(ui_cameraPanel, &CameraSettingPanel::requestUpdateDevice, this, &SettingView::requestUpdateCameraDevice);
    connect(ui_cameraPanel, &CameraSettingPanel::requestUpdateDevice, this, [=](const QCameraDevice& device) {
        emit requestUpdateCamera(device, ui_cameraPanel->format());
    });
    connect(ui_cameraPanel, &CameraSettingPanel::requestUpdateFormat, this, &SettingView::requestUpdateCameraFormat);

    connect(ui_mqttPanel, &MqttSettingPanel::requestConnect, this, &SettingView::requestStartMqtt);
    connect(ui_mqttPanel, &MqttSettingPanel::requestDisconnect, this, &SettingView::requestStopMqtt);
}
