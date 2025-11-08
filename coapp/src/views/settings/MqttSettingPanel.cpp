#include "MqttSettingPanel.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>

#include "components/IPv4Edit.h"
#include "services/SettingsManager.h"

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
            m_idEdit->text(), m_usernameEdit->text(), m_passwordEdit->text());
    }
    else if (m_connectBtn->text() == tr("Disconnect")) {
        m_connectBtn->setText(tr("Disconnecting..."));
        m_connectBtn->setEnabled(false);
        emit requestDisconnect();
    }
}
