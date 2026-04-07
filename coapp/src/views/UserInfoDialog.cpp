#include "UserInfoDialog.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonValue>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

#include "services/AuthService.h"
#include "services/UserApi.h"

UserInfoDialog::UserInfoDialog(QWidget* parent)
    : QDialog(parent) {
    initUI();

    connect(&UserApi::instance(), &UserApi::currentUserFetched, this, &UserInfoDialog::onCurrentUserFetched);
    connect(&UserApi::instance(), &UserApi::currentUserFetchFailed, this, &UserInfoDialog::onCurrentUserFetchFailed);
    connect(m_logoutButton, &QPushButton::clicked, this, &UserInfoDialog::onLogoutClicked);
    connect(&AuthService::instance(), &AuthService::authenticationChanged, this, [this](const bool authenticated) {
        if (!authenticated)
            close();
    });
}

void UserInfoDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    loadUserInfo();
}

void UserInfoDialog::onCurrentUserFetched(const QJsonObject& userInfo) {
    m_hasLoadedOnce = true;
    m_statusLabel->setText(tr("User information has been updated."));
    m_statusLabel->setProperty("error", false);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);

    setFieldValue(m_nameValueLabel, userInfo.value("name").toString());
    setFieldValue(m_idValueLabel, userInfo.value("unified_id").toString());
    setFieldValue(m_typeValueLabel, userInfo.value("user_type").toString());

    const QJsonValue activeValue = userInfo.value("is_active");
    QString activeText = tr("Unknown");
    if (activeValue.isBool())
        activeText = activeValue.toBool() ? tr("Active") : tr("Inactive");
    setFieldValue(m_activeValueLabel, activeText);

    m_profileView->setPlainText(QString::fromUtf8(QJsonDocument(userInfo).toJson(QJsonDocument::Indented)));
}

void UserInfoDialog::onCurrentUserFetchFailed(const int statusCode, const QString& message) {
    const QString detail = message.isEmpty()
        ? tr("Failed to fetch user information.")
        : tr("Failed to fetch user information: %1").arg(message);
    m_statusLabel->setText(statusCode > 0 ? tr("Request failed (%1) %2").arg(statusCode).arg(detail) : detail);
    m_statusLabel->setProperty("error", true);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);

    if (!m_hasLoadedOnce) {
        setFieldValue(m_nameValueLabel, {});
        setFieldValue(m_idValueLabel, {});
        setFieldValue(m_typeValueLabel, {});
        setFieldValue(m_activeValueLabel, {});
        m_profileView->clear();
    }
}

void UserInfoDialog::onLogoutClicked() {
    m_logoutButton->setEnabled(false);
    m_statusLabel->setText(tr("Signing out..."));
    m_statusLabel->setProperty("error", false);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
    AuthService::logout();
}

void UserInfoDialog::initUI() {
    setModal(true);
    setWindowTitle(tr("User Information"));
    resize(520, 420);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(18, 18, 18, 18);
    rootLayout->setSpacing(12);

    auto* titleLabel = new QLabel(tr("Current User"), this);
    titleLabel->setObjectName("dialogTitle");

    m_statusLabel = new QLabel(tr("Ready to fetch user information."), this);
    m_statusLabel->setObjectName("dialogHint");
    m_statusLabel->setWordWrap(true);

    auto* formLayout = new QFormLayout;
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setHorizontalSpacing(18);
    formLayout->setVerticalSpacing(10);

    m_nameValueLabel = new QLabel(this);
    m_idValueLabel = new QLabel(this);
    m_typeValueLabel = new QLabel(this);
    m_activeValueLabel = new QLabel(this);

    formLayout->addRow(tr("Name"), m_nameValueLabel);
    formLayout->addRow(tr("Unified Identity ID"), m_idValueLabel);
    formLayout->addRow(tr("User Type"), m_typeValueLabel);
    formLayout->addRow(tr("Account Status"), m_activeValueLabel);

    m_profileView = new QPlainTextEdit(this);
    m_profileView->setReadOnly(true);
    m_profileView->setPlaceholderText(tr("The complete user information will be displayed here."));

    m_logoutButton = new QPushButton(tr("Sign Out"), this);
    m_logoutButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));

    auto* actionLayout = new QHBoxLayout;
    actionLayout->addStretch(1);
    actionLayout->addWidget(m_logoutButton);

    rootLayout->addWidget(titleLabel);
    rootLayout->addWidget(m_statusLabel);
    rootLayout->addLayout(formLayout);
    rootLayout->addWidget(m_profileView, 1);
    rootLayout->addLayout(actionLayout);

    setFieldValue(m_nameValueLabel, {});
    setFieldValue(m_idValueLabel, {});
    setFieldValue(m_typeValueLabel, {});
    setFieldValue(m_activeValueLabel, {});
}

void UserInfoDialog::loadUserInfo() {
    m_logoutButton->setEnabled(true);
    m_statusLabel->setText(tr("Fetching user information..."));
    m_statusLabel->setProperty("error", false);
    m_statusLabel->style()->unpolish(m_statusLabel);
    m_statusLabel->style()->polish(m_statusLabel);
    UserApi::fetchCurrentUser();
}

void UserInfoDialog::setFieldValue(QLabel* label, const QString& value) {
    if (!label)
        return;
    label->setText(value.trimmed().isEmpty() ? tr("Not provided") : value);
}
