#include "SettingView.h"

#include <QtCore/QStringList>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

#include "settings/BandSettingPanel.h"
#include "settings/CameraSettingPanel.h"
#include "settings/EEGSettingPanel.h"
#include "settings/InfoPanel.h"
#include "settings/StreamSettingPanel.h"

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

void SettingView::onMqttConnected() {
    m_mqttConnected = true;
    m_mqttShowStatusCard = false;
    updateMqttStatusCard();
}

void SettingView::onMqttDisconnected() {
    m_mqttConnected = false;
    m_mqttShowStatusCard = true;
    updateMqttStatusCard();
}

void SettingView::onMqttError(const QString& message) {
    Q_UNUSED(message)
    m_mqttShowStatusCard = true;
    updateMqttStatusCard();
}

void SettingView::onVideoPushStateChanged(const PushWorkerState state) const {
    ui_streamPanel->handleStateChanged(state);
}

void SettingView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateMqttStatusCardLayout();
}

void SettingView::initUI() {
    ui_eegPanel = new EEGSettingPanel();
    ui_bandPanel = new BandSettingPanel();
    ui_cameraPanel = new CameraSettingPanel();
    ui_streamPanel = new StreamSettingPanel();
    ui_infoPanel = new InfoPanel();
    ui_mqttStatusCard = new QFrame();
    ui_mqttStatusCard->setObjectName("mqttWarningCard");
    ui_mqttStatusTitleLabel = new QLabel(tr("Message Push Service Connection Failed"), ui_mqttStatusCard);
    ui_mqttStatusTitleLabel->setObjectName("mqttWarningTitle");
    ui_mqttStatusTitleLabel->setWordWrap(true);
    ui_mqttStatusBodyLabel = new QLabel(ui_mqttStatusCard);
    ui_mqttStatusBodyLabel->setObjectName("mqttWarningBody");
    ui_mqttStatusBodyLabel->setWordWrap(true);
    ui_mqttReconnectButton = new QPushButton(tr("Force Retry"), ui_mqttStatusCard);
    ui_mqttReconnectButton->setObjectName("primary");

    auto* mqttCardLayout = new QVBoxLayout(ui_mqttStatusCard);
    mqttCardLayout->setContentsMargins(14, 14, 14, 14);
    mqttCardLayout->setSpacing(10);
    mqttCardLayout->addWidget(ui_mqttStatusTitleLabel);
    mqttCardLayout->addWidget(ui_mqttStatusBodyLabel);
    mqttCardLayout->addWidget(ui_mqttReconnectButton);

    auto* container = new QWidget();
    container->setMinimumWidth(250);
    ui_infoPanel->setMinimumWidth(250);
    auto* containerLayout = new QVBoxLayout(container);
    containerLayout->setSpacing(8);
    containerLayout->setContentsMargins(0, 0, 2, 0);
    containerLayout->addWidget(ui_eegPanel);
    containerLayout->addWidget(ui_bandPanel);
    containerLayout->addWidget(ui_cameraPanel);
    containerLayout->addWidget(ui_streamPanel);
    containerLayout->addWidget(ui_mqttStatusCard);
    containerLayout->addStretch(1);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidget(container);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFrameShape(QFrame::NoFrame);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(scrollArea);
    layout->addWidget(ui_infoPanel);

    updateMqttStatusCard();
    updateMqttStatusCardLayout();
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
    connect(ui_mqttReconnectButton, &QPushButton::clicked, this, &SettingView::requestReconnectMqtt);

    connect(ui_streamPanel, &StreamSettingPanel::requestStart, this, &SettingView::requestStartVideoPush);
    connect(ui_streamPanel, &StreamSettingPanel::requestStop, this, &SettingView::requestStopVideoPush);
}

void SettingView::updateMqttStatusCard() {
    if (!ui_mqttStatusCard)
        return;

    ui_mqttStatusCard->setVisible(m_mqttShowStatusCard && !m_mqttConnected);
    if (!m_mqttShowStatusCard || m_mqttConnected)
        return;

    ui_mqttStatusBodyLabel->setText(
        tr("Please check whether the RabbitMQ service and related backend services have been started.\n"
            "App will try to reconnect the service.")
    );
}

void SettingView::updateMqttStatusCardLayout() {
    if (!ui_mqttStatusCard || !ui_mqttReconnectButton)
        return;

    ui_mqttReconnectButton->setSizePolicy(
        ui_mqttStatusCard->width() < 280 ? QSizePolicy::Expanding : QSizePolicy::Preferred,
        QSizePolicy::Preferred);
}
