#include "BandSettingPanel.h"

#include <QFormLayout>
#include <QPushButton>
#include <QSpinBox>

#include "services/SettingsManager.h"

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
    }
    else {
        handleServiceStopped();
    }
}

void BandSettingPanel::onListenBtnClicked() {
    if (m_listenBtn->text() == tr("Start Service")) {
        m_listenBtn->setText(tr("Starting..."));
        m_listenBtn->setEnabled(false);
        m_portSpinBox->setEnabled(false);
        emit requestStartService(m_portSpinBox->value());
    }
    else if (m_listenBtn->text() == tr("Stop Service")) {
        m_listenBtn->setText(tr("Stopping..."));
        m_listenBtn->setEnabled(false);
        emit requestStopService();
    }
}
