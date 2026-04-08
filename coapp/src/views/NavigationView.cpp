#include "NavigationView.h"

#include <QtCore/QStringList>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

#include "navigation/BandSettingPanel.h"
#include "navigation/CameraSettingPanel.h"
#include "navigation/EEGSettingPanel.h"
#include "navigation/InfoPanel.h"
#include "navigation/StreamSettingPanel.h"

NavigationView::NavigationView(QWidget* parent)
    : QWidget(parent) {
    initUI();
    initConnection();
}

QString NavigationView::eegAddress() const {
    return ui_eegPanel->address();
}

int NavigationView::eegPort() const {
    return ui_eegPanel->port();
}

int NavigationView::bandServicePort() const {
    return ui_bandPanel->port();
}

QCameraDevice NavigationView::cameraDevice() const {
    return ui_cameraPanel->device();
}

QCameraFormat NavigationView::cameraFormat() const {
    return ui_cameraPanel->format();
}

void NavigationView::onEEGConnected() const {
    ui_eegPanel->handleConnected();
}

void NavigationView::onEEGDisconnected() const {
    ui_eegPanel->handleDisconnected();
}

void NavigationView::onEEGReceiverRunningChanged(const bool connected) const {
    if (connected) {
        onEEGConnected();
    } else {
        onEEGDisconnected();
    }
}

void NavigationView::onBandServiceStarted() const {
    ui_bandPanel->handleServiceStarted();
}

void NavigationView::onBandServiceStopped() const {
    ui_bandPanel->handleServiceStopped();
}

void NavigationView::onBandServiceRunningChanged(const bool running) const {
    ui_bandPanel->handleServiceRunningChanged(running);
}

void NavigationView::onCameraOpened() const {
    ui_cameraPanel->handleOpened();
}

void NavigationView::onCameraClosed() const {
    ui_cameraPanel->handleClosed();
}

void NavigationView::onCameraRunningChanged(const bool running) const {
    ui_cameraPanel->handleRunningChanged(running);
}

void NavigationView::onMqttConnected() {
    m_mqttConnected = true;
    m_mqttShowStatusCard = false;
    updateMqttStatusCard();
}

void NavigationView::onMqttDisconnected() {
    m_mqttConnected = false;
    m_mqttShowStatusCard = true;
    updateMqttStatusCard();
}

void NavigationView::onMqttError(const QString& message) {
    Q_UNUSED(message)
    m_mqttShowStatusCard = true;
    updateMqttStatusCard();
}

void NavigationView::onVideoPushStateChanged(const PushWorkerState state) const {
    ui_streamPanel->handleStateChanged(state);
}

void NavigationView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateMqttStatusCardLayout();
}

void NavigationView::initUI() {
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

void NavigationView::initConnection() {
    connect(ui_eegPanel, &EEGSettingPanel::requestConnect, this, &NavigationView::requestConnectEEG);
    connect(ui_eegPanel, &EEGSettingPanel::requestDisconnect, this, &NavigationView::requestDisconnectEEG);

    connect(ui_bandPanel, &BandSettingPanel::requestStartService, this, &NavigationView::requestStartBandService);
    connect(ui_bandPanel, &BandSettingPanel::requestStopService, this, &NavigationView::requestStopBandService);

    connect(ui_cameraPanel, &CameraSettingPanel::requestOpen, this, &NavigationView::requestOpenCamera);
    connect(ui_cameraPanel, &CameraSettingPanel::requestClose, this, &NavigationView::requestCloseCamera);
    connect(ui_cameraPanel, &CameraSettingPanel::requestUpdateDevice, this, &NavigationView::requestUpdateCameraDevice);
    connect(ui_cameraPanel, &CameraSettingPanel::requestUpdateDevice, this, [=](const QCameraDevice& device) {
        emit requestUpdateCamera(device, ui_cameraPanel->format());
    });
    connect(ui_cameraPanel, &CameraSettingPanel::requestUpdateFormat, this, &NavigationView::requestUpdateCameraFormat);
    connect(ui_mqttReconnectButton, &QPushButton::clicked, this, &NavigationView::requestReconnectMqtt);

    connect(ui_streamPanel, &StreamSettingPanel::requestStart, this, &NavigationView::requestStartVideoPush);
    connect(ui_streamPanel, &StreamSettingPanel::requestStop, this, &NavigationView::requestStopVideoPush);
}

void NavigationView::updateMqttStatusCard() {
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

void NavigationView::updateMqttStatusCardLayout() {
    if (!ui_mqttStatusCard || !ui_mqttReconnectButton)
        return;

    ui_mqttReconnectButton->setSizePolicy(
        ui_mqttStatusCard->width() < 280 ? QSizePolicy::Expanding : QSizePolicy::Preferred,
        QSizePolicy::Preferred);
}
