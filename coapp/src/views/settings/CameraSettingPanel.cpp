#include "CameraSettingPanel.h"

#include <QComboBox>
#include <QFormLayout>
#include <QPushButton>

namespace {
    QString format2String(const QCameraFormat& cameraFormat) {
        return QString("%1×%2@%3fps, %4")
               .arg(cameraFormat.resolution().width())
               .arg(cameraFormat.resolution().height())
               .arg(cameraFormat.maxFrameRate())
               .arg(QVideoFrameFormat::pixelFormatToString(cameraFormat.pixelFormat()));
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
    }
    else {
        handleClosed();
    }
}

void CameraSettingPanel::onOpenBtnClicked() {
    if (m_openBtn->text() == tr("Open")) {
        m_openBtn->setText(tr("Opening..."));
        m_openBtn->setEnabled(false);
        emit requestOpen(m_device, m_format);
    }
    else if (m_openBtn->text() == tr("Close")) {
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
    for (const auto& device : allDevices) {
        m_deviceComboBox->addItem(device.description(), QVariant::fromValue(device));
        isOldExist = isOldExist || (device == m_device);
    }

    if (isOldExist) {
        m_deviceComboBox->setCurrentText(m_device.description());
    }
    else {
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
        for (const auto& format : allFormats) {
            m_formatComboBox->addItem(format2String(format), QVariant::fromValue(format));
        }
    }
    updateCurrentFormat();

    connect(m_formatComboBox, &QComboBox::currentTextChanged, this, &CameraSettingPanel::updateCurrentFormat);
}

void CameraSettingPanel::updateOpenBtn() const {
    if (m_device.isNull()) {
        m_openBtn->setEnabled(false);
    }
    else {
        m_openBtn->setEnabled(true);
    }
}
