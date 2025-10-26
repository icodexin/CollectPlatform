#include "SettingView.h"

#include <QPushButton>
#include <QSpinBox>
#include <QComboBox>
#include <QMessageBox>
#include <QFormLayout>
#include <QLineEdit>

#include "components/IPv4Edit.h"
#include "services/SettingsManager.h"

namespace {
    QString format2String(const QCameraFormat& cameraFormat) {
        return QString("%1×%2@%3fps, %4")
                .arg(cameraFormat.resolution().width())
                .arg(cameraFormat.resolution().height())
                .arg(cameraFormat.maxFrameRate())
                .arg(QVideoFrameFormat::pixelFormatToString(cameraFormat.pixelFormat()));
    }
}

EEGSettingPanel::EEGSettingPanel(QWidget* parent)
    : QGroupBox(tr("EEG"), parent) {
    m_addressEdit = new IPv4Edit();
    m_addressEdit->setAddress(SettingsManager::eegAddress());
    m_addressEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_portSpinBox = new QSpinBox();
    m_portSpinBox->setAlignment(Qt::AlignCenter);
    m_portSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_portSpinBox->setRange(1024, 65535);
    m_portSpinBox->setValue(SettingsManager::eegPort());
    m_connectBtn = new QPushButton(tr("Connect"));

    auto* layout = new QFormLayout(this);
    layout->setSpacing(4);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layout->addRow(tr("Host"), m_addressEdit);
    layout->addRow(tr("Port"), m_portSpinBox);
    layout->addRow(m_connectBtn);

    connect(m_connectBtn, &QPushButton::clicked, this, &EEGSettingPanel::onConnectBtnClicked);
    connect(m_addressEdit, &IPv4Edit::addressChanged, this, [=](const QString& address) {
        SettingsManager::setEEGAddress(address);
    });
    connect(m_portSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, [=](const int port) {
        SettingsManager::setEEGPort(port);
    });
}

QString EEGSettingPanel::address() const {
    return m_addressEdit->address();
}

int EEGSettingPanel::port() const {
    return m_portSpinBox->value();
}

void EEGSettingPanel::handleConnected() const {
    m_connectBtn->setText(tr("Disconnect"));
    m_connectBtn->setEnabled(true);
}

void EEGSettingPanel::handleDisconnected() const {
    m_addressEdit->setEnabled(true);
    m_portSpinBox->setEnabled(true);
    m_connectBtn->setText(tr("Connect"));
    m_connectBtn->setEnabled(true);
}

void EEGSettingPanel::onConnectBtnClicked() {
    if (m_connectBtn->text() == tr("Connect")) {
        if (m_addressEdit->address().isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please enter a valid EEG address."));
            return;
        }
        m_connectBtn->setText(tr("Connecting..."));
        m_connectBtn->setEnabled(false);
        m_addressEdit->setEnabled(false);
        m_portSpinBox->setEnabled(false);
        emit requestConnect(m_addressEdit->address(), m_portSpinBox->value());
    } else if (m_connectBtn->text() == tr("Disconnect")) {
        m_connectBtn->setText(tr("Disconnecting..."));
        m_connectBtn->setEnabled(false);
        emit requestDisconnect();
    }
}

BandSettingPanel::BandSettingPanel(QWidget* parent)
    : QGroupBox(tr("Wristband"), parent) {
    m_portSpinBox = new QSpinBox();
    m_portSpinBox->setAlignment(Qt::AlignCenter);
    m_portSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_portSpinBox->setRange(1024, 65535);
    m_portSpinBox->setValue(SettingsManager::bandPort());
    m_listenBtn = new QPushButton(tr("Start Service"));

    auto* layout = new QFormLayout(this);
    layout->setSpacing(4);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layout->addRow(tr("Port"), m_portSpinBox);
    layout->addRow(m_listenBtn);

    connect(m_listenBtn, &QPushButton::clicked, this, &BandSettingPanel::onListenBtnClicked);
    connect(m_portSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, [=](const int port) {
        SettingsManager::setBandPort(port);
    });
}

int BandSettingPanel::port() const {
    return m_portSpinBox->value();
}

void BandSettingPanel::handleServiceStarted() const {
    m_listenBtn->setText(tr("Stop Service"));
    m_listenBtn->setEnabled(true);
}

void BandSettingPanel::handleServiceStopped() const {
    m_portSpinBox->setEnabled(true);
    m_listenBtn->setText(tr("Start Service"));
    m_listenBtn->setEnabled(true);
}

void BandSettingPanel::handleServiceRunningChanged(const bool running) const {
    if (running) {
        handleServiceStarted();
    } else {
        handleServiceStopped();
    }
}

void BandSettingPanel::onListenBtnClicked() {
    if (m_listenBtn->text() == tr("Start Service")) {
        m_listenBtn->setText(tr("Starting..."));
        m_listenBtn->setEnabled(false);
        m_portSpinBox->setEnabled(false);
        emit requestStartService(m_portSpinBox->value());
    } else if (m_listenBtn->text() == tr("Stop Service")) {
        m_listenBtn->setText(tr("Stopping..."));
        m_listenBtn->setEnabled(false);
        emit requestStopService();
    }
}

CameraSettingPanel::CameraSettingPanel(QWidget* parent)
    : QGroupBox(tr("Camera"), parent) {
    m_deviceComboBox = new QComboBox();
    m_deviceComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_formatComboBox = new QComboBox();
    m_formatComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_openBtn = new QPushButton(tr("Open"));

    auto* layout = new QFormLayout(this);
    layout->setSpacing(4);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layout->addRow(tr("Device"), m_deviceComboBox);
    layout->addRow(tr("Format"), m_formatComboBox);
    layout->addRow(m_openBtn);

    connect(m_openBtn, &QPushButton::clicked, this, &CameraSettingPanel::onOpenBtnClicked);
    connect(&m_mediaDevices, &QMediaDevices::videoInputsChanged, this, &CameraSettingPanel::updateDeviceComboBox);
    connect(this, &CameraSettingPanel::requestUpdateDevice, this, &CameraSettingPanel::updateFormatComboBox);
    connect(this, &CameraSettingPanel::requestUpdateDevice, this, &CameraSettingPanel::updateOpenBtn);
    connect(m_deviceComboBox, &QComboBox::currentTextChanged, this, &CameraSettingPanel::updateCurrentDevice);
    connect(m_formatComboBox, &QComboBox::currentTextChanged, this, &CameraSettingPanel::updateCurrentFormat);
    updateDeviceComboBox();
    updateFormatComboBox();
    updateOpenBtn();
}

QCameraDevice CameraSettingPanel::device() const {
    return m_device;
}

QCameraFormat CameraSettingPanel::format() const {
    return m_format;
}

void CameraSettingPanel::handleOpened() const {
    m_openBtn->setText(tr("Close"));
    m_openBtn->setEnabled(true);
}

void CameraSettingPanel::handleClosed() const {
    m_openBtn->setText(tr("Open"));
    m_openBtn->setEnabled(true);
}

void CameraSettingPanel::handleRunningChanged(const bool running) const {
    if (running) {
        handleOpened();
    } else {
        handleClosed();
    }
}

void CameraSettingPanel::onOpenBtnClicked() {
    if (m_openBtn->text() == tr("Open")) {
        m_openBtn->setText(tr("Opening..."));
        m_openBtn->setEnabled(false);
        emit requestOpen(m_device, m_format);
    } else if (m_openBtn->text() == tr("Close")) {
        m_openBtn->setText(tr("Closing..."));
        m_openBtn->setEnabled(false);
        emit requestClose();
    }
}

void CameraSettingPanel::updateCurrentDevice() {
    setDevice(qvariant_cast<QCameraDevice>(m_deviceComboBox->currentData()));
}

void CameraSettingPanel::updateCurrentFormat() {
    setFormat(qvariant_cast<QCameraFormat>(m_formatComboBox->currentData()));
}

void CameraSettingPanel::setDevice(const QCameraDevice& device) {
    if (m_device == device)
        return;
    m_device = device;
    emit requestUpdateDevice(device);
}

void CameraSettingPanel::setFormat(const QCameraFormat& format) {
    if (m_format == format)
        return;
    m_format = format;
    emit requestUpdateFormat(format);
}

void CameraSettingPanel::updateDeviceComboBox() {
    disconnect(m_deviceComboBox, &QComboBox::currentTextChanged, this, &CameraSettingPanel::updateCurrentDevice);

    m_deviceComboBox->clear();
    QList<QCameraDevice> allDevices = QMediaDevices::videoInputs();
    bool isOldExist = false;
    for (const auto& device: allDevices) {
        m_deviceComboBox->addItem(device.description(), QVariant::fromValue(device));
        isOldExist = isOldExist || (device == m_device);
    }

    if (isOldExist) {
        m_deviceComboBox->setCurrentText(m_device.description());
    } else {
        const QCameraDevice defaultDevice = QMediaDevices::defaultVideoInput();
        if (!defaultDevice.isNull()) {
            m_deviceComboBox->setCurrentText(defaultDevice.description());
        }
    }
    updateCurrentDevice();

    connect(m_deviceComboBox, &QComboBox::currentTextChanged, this, &CameraSettingPanel::updateCurrentDevice);
}

void CameraSettingPanel::updateFormatComboBox() {
    disconnect(m_formatComboBox, &QComboBox::currentTextChanged, this, &CameraSettingPanel::updateCurrentFormat);

    m_formatComboBox->clear();
    if (!m_device.isNull()) {
        auto allFormats = m_device.videoFormats();
        for (const auto& format: allFormats) {
            m_formatComboBox->addItem(format2String(format), QVariant::fromValue(format));
        }
    }
    updateCurrentFormat();

    connect(m_formatComboBox, &QComboBox::currentTextChanged, this, &CameraSettingPanel::updateCurrentFormat);
}

void CameraSettingPanel::updateOpenBtn() const {
    if (m_device.isNull()) {
        m_openBtn->setEnabled(false);
    } else {
        m_openBtn->setEnabled(true);
    }
}

MqttSettingPanel::MqttSettingPanel(QWidget* parent) : QGroupBox(tr("MQTT"), parent) {
    m_addressEdit = new IPv4Edit();
    m_addressEdit->setAddress(SettingsManager::mqttAddress());
    m_addressEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_portSpinBox = new QSpinBox();
    m_portSpinBox->setAlignment(Qt::AlignCenter);
    m_portSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_portSpinBox->setRange(1024, 65535);
    m_portSpinBox->setValue(SettingsManager::mqttPort());
    m_idEdit = new QLineEdit();
    m_idEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_usernameEdit = new QLineEdit();
    m_usernameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_passwordEdit = new QLineEdit();
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_connectBtn = new QPushButton(tr("Connect"));

    auto* layout = new QFormLayout(this);
    layout->setSpacing(4);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layout->addRow(tr("Host"), m_addressEdit);
    layout->addRow(tr("Port"), m_portSpinBox);
    layout->addRow(tr("ID"), m_idEdit);
    layout->addRow(tr("Username"), m_usernameEdit);
    layout->addRow(tr("Password"), m_passwordEdit);
    layout->addRow(m_connectBtn);

    connect(m_connectBtn, &QPushButton::clicked, this, &MqttSettingPanel::onConnectBtnClicked);
    connect(m_addressEdit, &IPv4Edit::addressChanged, this, [=](const QString& address) {
        SettingsManager::setMQTTAddress(address);
    });
    connect(m_portSpinBox, qOverload<int>(&QSpinBox::valueChanged), this, [=](const int port) {
        SettingsManager::setMQTTPort(port);
    });
}

QString MqttSettingPanel::address() const {
    return m_addressEdit->address();
}

int MqttSettingPanel::port() const {
    return m_portSpinBox->value();
}

QString MqttSettingPanel::id() const {
    return m_idEdit->text();
}

QString MqttSettingPanel::username() const {
    return m_usernameEdit->text();
}

QString MqttSettingPanel::password() const {
    return m_passwordEdit->text();
}

void MqttSettingPanel::handleConnected() const {
    m_connectBtn->setText(tr("Disconnect"));
    m_connectBtn->setEnabled(true);
}

void MqttSettingPanel::handleDisconnected() const {
    m_addressEdit->setEnabled(true);
    m_portSpinBox->setEnabled(true);
    m_idEdit->setEnabled(true);
    m_usernameEdit->setEnabled(true);
    m_passwordEdit->setEnabled(true);
    m_connectBtn->setText(tr("Connect"));
    m_connectBtn->setEnabled(true);
}

void MqttSettingPanel::onConnectBtnClicked() {
    if (m_connectBtn->text() == tr("Connect")) {
        if (m_addressEdit->address().isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please enter a valid MQTT address."));
            return;
        }
        if (m_idEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please enter a valid MQTT client ID."));
            return;
        }
        if (m_usernameEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please enter a valid MQTT username."));
            return;
        }
        if (m_passwordEdit->text().isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please enter a valid MQTT password."));
            return;
        }
        m_connectBtn->setText(tr("Connecting..."));
        m_connectBtn->setEnabled(false);
        m_addressEdit->setEnabled(false);
        m_portSpinBox->setEnabled(false);
        m_idEdit->setEnabled(false);
        m_usernameEdit->setEnabled(false);
        m_passwordEdit->setEnabled(false);
        emit requestConnect(m_addressEdit->address(), m_portSpinBox->value(),
                            m_idEdit->text(),  m_usernameEdit->text(), m_passwordEdit->text());
    } else if (m_connectBtn->text() == tr("Disconnect")) {
        m_connectBtn->setText(tr("Disconnecting..."));
        m_connectBtn->setEnabled(false);
        emit requestDisconnect();
    }
}

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

    auto* layout = new QVBoxLayout(this);
    layout->setSpacing(8);
    layout->addWidget(ui_eegPanel);
    layout->addWidget(ui_bandPanel);
    layout->addWidget(ui_cameraPanel);
    layout->addWidget(ui_mqttPanel);
    layout->addStretch(1);
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
