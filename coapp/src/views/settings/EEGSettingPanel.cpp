#include "EEGSettingPanel.h"

#include <QFormLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>

#include "components/IPv4Edit.h"
#include "services/CoSettingsMgr.h"

EEGSettingPanel::EEGSettingPanel(QWidget* parent)
    : QGroupBox(tr("EEG"), parent) {
    m_addressEdit = new IPv4Edit();
    m_addressEdit->setAddress(CoSettingsMgr::eegAddress());
    m_addressEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_portSpinBox = new QSpinBox();
    m_portSpinBox->setAlignment(Qt::AlignCenter);
    m_portSpinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_portSpinBox->setRange(1024, 65535);
    m_portSpinBox->setValue(CoSettingsMgr::eegPort());
    m_connectBtn = new QPushButton(tr("Connect"));

    auto* layout = new QFormLayout(this);
    layout->setSpacing(4);
    layout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    layout->addRow(tr("Host"), m_addressEdit);
    layout->addRow(tr("Port"), m_portSpinBox);
    layout->addRow(m_connectBtn);

    connect(m_connectBtn, &QPushButton::clicked, this, &EEGSettingPanel::onConnectBtnClicked);
    connect(m_addressEdit, &IPv4Edit::addressChanged, this,
        [=](const QString& address) { CoSettingsMgr::setEEGAddress(address); });
    connect(m_portSpinBox, qOverload<int>(&QSpinBox::valueChanged), this,
        [=](const int port) { CoSettingsMgr::setEEGPort(port); });
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
    }
    else if (m_connectBtn->text() == tr("Disconnect")) {
        m_connectBtn->setText(tr("Disconnecting..."));
        m_connectBtn->setEnabled(false);
        emit requestDisconnect();
    }
}
