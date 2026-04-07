#include "SettingsDialog.h"

#include <QtWidgets/QComboBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

#include "network/HttpMgr.h"
#include "services/CoSettingsMgr.h"

namespace {
    void updateHintLabel(QLabel* label, const QString& message, const bool error) {
        if (!label)
            return;

        label->setVisible(!message.isEmpty());
        label->setProperty("error", error);
        label->setText(message);
        label->style()->unpolish(label);
        label->style()->polish(label);
    }
}

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent) {
    initUI();

    connect(m_saveButton, &QPushButton::clicked, this, &SettingsDialog::onSaveClicked);
    connect(m_serverAddressEdit, &QLineEdit::returnPressed, this, &SettingsDialog::onSaveClicked);
}

void SettingsDialog::onSaveClicked() {
    const QString host = m_serverAddressEdit->text().trimmed();
    if (host.isEmpty()) {
        setStatusMessage(tr("Need server address."), true);
        return;
    }

    const QUrl url = HttpHelper::buildApiBaseUrl(host);
    if (!url.isValid()) {
        setStatusMessage(tr("The server address format is invalid."), true);
        return;
    }

    CoSettingsMgr::setServerHostname(host);
    CoSettingsMgr::flush();
    HttpMgr::setBaseUrl(url);

    setStatusMessage(tr("Server address saved."));
}

void SettingsDialog::initUI() {
    setModal(true);
    setWindowTitle(tr("Settings"));
    setMinimumSize(500, 450);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(20, 20, 20, 20);
    rootLayout->setSpacing(14);

    auto* titleLabel = new QLabel(tr("Application Settings"), this);
    titleLabel->setObjectName("dialogTitle");

    auto* subtitleLabel = new QLabel(tr("Configure the current application environment."), this);
    subtitleLabel->setObjectName("dialogSubtitle");
    subtitleLabel->setWordWrap(true);

    auto* networkCard = new QFrame(this);
    networkCard->setObjectName("settingsSectionCard");
    auto* networkLayout = new QVBoxLayout(networkCard);
    networkLayout->setContentsMargins(16, 16, 16, 16);
    networkLayout->setSpacing(12);

    auto* networkTitle = new QLabel(tr("Network"), networkCard);
    networkTitle->setObjectName("settingsSectionTitle");

    auto* networkSubtitle = new QLabel(
        tr("Set the server address used by app."),
        networkCard);
    networkSubtitle->setObjectName("dialogSubtitle");
    networkSubtitle->setWordWrap(true);

    auto* networkFormLayout = new QFormLayout;
    networkFormLayout->setContentsMargins(0, 0, 0, 0);
    networkFormLayout->setHorizontalSpacing(16);
    networkFormLayout->setVerticalSpacing(10);
    networkFormLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_serverAddressEdit = new QLineEdit(networkCard);
    m_serverAddressEdit->setPlaceholderText(tr("Server hostname / IP"));
    m_serverAddressEdit->setClearButtonEnabled(true);
    m_serverAddressEdit->setText(CoSettingsMgr::serverHostname());

    networkFormLayout->addRow(tr("Server Address"), m_serverAddressEdit);

    m_statusLabel = new QLabel(networkCard);
    m_statusLabel->setObjectName("dialogHint");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setVisible(false);

    networkLayout->addWidget(networkTitle);
    networkLayout->addWidget(networkSubtitle);
    networkLayout->addLayout(networkFormLayout);
    networkLayout->addWidget(m_statusLabel);

    auto* appearanceCard = new QFrame(this);
    appearanceCard->setObjectName("settingsSectionCard");
    auto* appearanceLayout = new QVBoxLayout(appearanceCard);
    appearanceLayout->setContentsMargins(16, 16, 16, 16);
    appearanceLayout->setSpacing(12);

    auto* appearanceTitle = new QLabel(tr("Appearance"), appearanceCard);
    appearanceTitle->setObjectName("settingsSectionTitle");

    auto* appearanceSubtitle = new QLabel(
        tr("Theme switching will be available in a future version."),
        appearanceCard);
    appearanceSubtitle->setObjectName("dialogSubtitle");
    appearanceSubtitle->setWordWrap(true);

    auto* appearanceFormLayout = new QFormLayout;
    appearanceFormLayout->setContentsMargins(0, 0, 0, 0);
    appearanceFormLayout->setHorizontalSpacing(16);
    appearanceFormLayout->setVerticalSpacing(10);
    appearanceFormLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    m_themeComboBox = new QComboBox(appearanceCard);
    m_themeComboBox->addItem(tr("Light"));
    m_themeComboBox->addItem(tr("Dark"));
    m_themeComboBox->setCurrentIndex(0);
    m_themeComboBox->setEnabled(false);
    m_themeComboBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    appearanceFormLayout->addRow(tr("Theme"), m_themeComboBox);

    appearanceLayout->addWidget(appearanceTitle);
    appearanceLayout->addWidget(appearanceSubtitle);
    appearanceLayout->addLayout(appearanceFormLayout);

    auto* actionLayout = new QHBoxLayout;
    actionLayout->setContentsMargins(0, 0, 0, 0);
    actionLayout->addStretch(1);

    auto* closeButton = new QPushButton(tr("Close"), this);
    closeButton->setAutoDefault(false);
    closeButton->setDefault(false);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::close);
    actionLayout->addWidget(closeButton);

    m_saveButton = new QPushButton(tr("Save"), this);
    m_saveButton->setObjectName("primary");
    m_saveButton->setAutoDefault(false);
    m_saveButton->setDefault(false);
    actionLayout->addWidget(m_saveButton);

    rootLayout->addStretch(1);
    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(subtitleLabel);
    rootLayout->addWidget(networkCard);
    rootLayout->addWidget(appearanceCard);
    rootLayout->addStretch(1);
    rootLayout->addLayout(actionLayout);
}

void SettingsDialog::setStatusMessage(const QString& message, const bool error) {
    updateHintLabel(m_statusLabel, message, error);
}
