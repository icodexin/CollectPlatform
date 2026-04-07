#include "LoginDialog.h"

#include <QtCore/QUrl>
#include <QtGui/QAction>
#include <QtGui/QDesktopServices>
#include <QtGui/QIcon>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyle>

#include "network/HttpMgr.h"
#include "services/AuthService.h"
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

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent) {
    initUI();

    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);
    connect(m_pageToggleButton, &QPushButton::clicked, this, &LoginDialog::togglePage);
    connect(m_saveNetworkButton, &QPushButton::clicked, this, &LoginDialog::onSaveNetworkClicked);
    connect(m_passwordEdit, &QLineEdit::returnPressed, this, &LoginDialog::onLoginClicked);
    connect(m_serverAddressEdit, &QLineEdit::returnPressed, this, &LoginDialog::onSaveNetworkClicked);
    connect(&AuthService::instance(), &AuthService::loginSucceeded, this, &LoginDialog::onLoginSucceeded);
    connect(&AuthService::instance(), &AuthService::loginFailed, this, &LoginDialog::onLoginFailed);
}

void LoginDialog::onLoginClicked() {
    if (m_busy)
        return;

    const QString username = m_usernameEdit->text().trimmed();
    const QString password = m_passwordEdit->text();
    if (username.isEmpty() || password.isEmpty()) {
        setStatusMessage(tr("Need username or password."), true);
        return;
    }

    setBusy(true);
    setStatusMessage(tr("Logging..."));
    AuthService::login(username, password);
}

void LoginDialog::onRegisterClicked() {
    const QUrl registerUrl = HttpMgr::baseUrl().resolved(QUrl(QStringLiteral("register")));
    if (!registerUrl.isValid() || registerUrl.isEmpty()) {
        setStatusMessage(tr("The registration link is invalid."), true);
        return;
    }

    if (!QDesktopServices::openUrl(registerUrl)) {
        setStatusMessage(tr("The registration page cannot be opened."), true);
    }
}

void LoginDialog::onSaveNetworkClicked() {
    const QString host = m_serverAddressEdit->text().trimmed();
    if (host.isEmpty()) {
        updateHintLabel(m_networkStatusLabel, tr("Need server address."), true);
        return;
    }

    const QUrl url = HttpHelper::buildApiBaseUrl(host);
    if (!url.isValid()) {
        updateHintLabel(m_networkStatusLabel, tr("The server address format is invalid."), true);
        return;
    }

    CoSettingsMgr::setServerHostname(host);
    CoSettingsMgr::flush();
    HttpMgr::setBaseUrl(url);

    updateHintLabel(m_networkStatusLabel, tr("Server address saved."), false);
}

void LoginDialog::togglePage() {
    if (!m_stackWidget || !m_loginPage || !m_networkPage)
        return;

    m_stackWidget->setCurrentWidget(m_stackWidget->currentWidget() == m_loginPage ? m_networkPage : m_loginPage);
    updatePageToggleButton();
}

void LoginDialog::onLoginSucceeded() {
    setBusy(false);
    accept();
}

void LoginDialog::onLoginFailed(const QString& message) {
    setBusy(false);
    setStatusMessage(message.isEmpty() ? tr("Login failed, please try again later.") : message, true);
}

void LoginDialog::initUI() {
    setModal(true);
    setWindowTitle(tr("User Login"));
    setObjectName("loginDialog");
    resize(420, 360);

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(28, 16, 28, 28);
    rootLayout->setSpacing(10);

    auto* topBarLayout = new QHBoxLayout;
    topBarLayout->setContentsMargins(0, 0, 0, 0);
    topBarLayout->addStretch(1);

    m_pageToggleButton = new QPushButton(this);
    m_pageToggleButton->setObjectName("linkButton");
    m_pageToggleButton->setFlat(true);
    m_pageToggleButton->setAutoDefault(false);
    m_pageToggleButton->setDefault(false);
    topBarLayout->addWidget(m_pageToggleButton, 0, Qt::AlignRight);

    auto* card = new QWidget(this);
    card->setObjectName("loginCard");
    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(28, 28, 28, 28);
    cardLayout->setSpacing(0);

    m_stackWidget = new QStackedWidget(card);
    initLoginPage();
    initNetworkPage();
    m_stackWidget->addWidget(m_loginPage);
    m_stackWidget->addWidget(m_networkPage);
    m_stackWidget->setCurrentWidget(m_loginPage);
    updatePageToggleButton();

    cardLayout->addWidget(m_stackWidget);

    rootLayout->addStretch(1);
    rootLayout->addLayout(topBarLayout);
    rootLayout->addWidget(card);
    rootLayout->addStretch(1);
}

void LoginDialog::initLoginPage() {
    m_loginPage = new QWidget(m_stackWidget);
    auto* cardLayout = new QVBoxLayout(m_loginPage);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(14);

    auto* logoLabel = new QLabel(m_loginPage);
    logoLabel->setPixmap(QIcon(":/res/icons/logo.svg").pixmap(56, 56));
    logoLabel->setMinimumHeight(64);
    logoLabel->setAlignment(Qt::AlignCenter);

    auto* titleLabel = new QLabel(qApp->applicationDisplayName(), m_loginPage);
    titleLabel->setObjectName("dialogTitle");
    titleLabel->setAlignment(Qt::AlignCenter);

    auto* subtitleLabel = new QLabel(
        tr("After logging in with a unified identity account, enter the main collection window."),
        m_loginPage);
    subtitleLabel->setObjectName("dialogSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);

    m_usernameEdit = new QLineEdit(m_loginPage);
    m_usernameEdit->setPlaceholderText(tr("Unified Identity ID"));
    m_usernameEdit->setClearButtonEnabled(true);

    m_passwordEdit = new QLineEdit(m_loginPage);
    m_passwordEdit->setPlaceholderText(tr("Password"));
    m_passwordEdit->setClearButtonEnabled(true);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_togglePasswordAction = m_passwordEdit->addAction(QIcon(), QLineEdit::TrailingPosition);
    connect(m_togglePasswordAction, &QAction::triggered, this, &LoginDialog::togglePasswordVisible);
    updatePasswordToggleIcon();

    m_statusLabel = new QLabel(m_loginPage);
    m_statusLabel->setObjectName("dialogHint");
    m_statusLabel->setWordWrap(true);
    m_statusLabel->setVisible(false);

    m_loginButton = new QPushButton(tr("Login"), m_loginPage);
    m_loginButton->setObjectName("primary");
    m_loginButton->setMinimumHeight(40);
    m_loginButton->setAutoDefault(false);
    m_loginButton->setDefault(false);

    m_registerButton = new QPushButton(tr("Don't have an account yet? Register Now"), m_loginPage);
    m_registerButton->setObjectName("linkButton");
    m_registerButton->setFlat(true);
    m_registerButton->setMinimumHeight(28);
    m_registerButton->setAutoDefault(false);
    m_registerButton->setDefault(false);

    cardLayout->addWidget(logoLabel);
    cardLayout->addWidget(titleLabel);
    cardLayout->addWidget(subtitleLabel);
    cardLayout->addSpacing(4);
    cardLayout->addWidget(m_usernameEdit);
    cardLayout->addWidget(m_passwordEdit);
    cardLayout->addWidget(m_statusLabel);
    cardLayout->addSpacing(4);
    cardLayout->addWidget(m_loginButton);
    cardLayout->addWidget(m_registerButton);
}

void LoginDialog::initNetworkPage() {
    m_networkPage = new QWidget(m_stackWidget);
    auto* pageLayout = new QVBoxLayout(m_networkPage);
    pageLayout->setContentsMargins(0, 0, 0, 0);
    pageLayout->setSpacing(14);

    auto* titleLabel = new QLabel(tr("Network Settings"), m_networkPage);
    titleLabel->setObjectName("dialogTitle");
    titleLabel->setAlignment(Qt::AlignCenter);

    auto* subtitleLabel = new QLabel(
        tr("Configure the server address."),
        m_networkPage);
    subtitleLabel->setObjectName("dialogSubtitle");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);

    m_serverAddressEdit = new QLineEdit(m_networkPage);
    m_serverAddressEdit->setPlaceholderText(tr("server hostname / ip"));
    m_serverAddressEdit->setClearButtonEnabled(true);
    m_serverAddressEdit->setText(CoSettingsMgr::serverHostname());

    m_networkStatusLabel = new QLabel(m_networkPage);
    m_networkStatusLabel->setObjectName("dialogHint");
    m_networkStatusLabel->setWordWrap(true);
    m_networkStatusLabel->setVisible(true);

    m_saveNetworkButton = new QPushButton(tr("Save Server Address"), m_networkPage);
    m_saveNetworkButton->setObjectName("primary");
    m_saveNetworkButton->setMinimumHeight(40);
    m_saveNetworkButton->setAutoDefault(false);
    m_saveNetworkButton->setDefault(false);

    pageLayout->addSpacing(8);
    pageLayout->addWidget(titleLabel);
    pageLayout->addWidget(subtitleLabel);
    pageLayout->addSpacing(10);
    pageLayout->addWidget(m_serverAddressEdit);
    pageLayout->addWidget(m_networkStatusLabel);
    pageLayout->addStretch(1);
    pageLayout->addWidget(m_saveNetworkButton);
}

void LoginDialog::setBusy(const bool busy) {
    m_busy = busy;
    if (m_usernameEdit)
        m_usernameEdit->setEnabled(!busy);
    if (m_passwordEdit)
        m_passwordEdit->setEnabled(!busy);
    if (m_loginButton)
        m_loginButton->setEnabled(!busy);
    if (m_registerButton)
        m_registerButton->setEnabled(!busy);
    if (m_pageToggleButton)
        m_pageToggleButton->setEnabled(!busy);
    if (m_serverAddressEdit)
        m_serverAddressEdit->setEnabled(!busy);
    if (m_saveNetworkButton)
        m_saveNetworkButton->setEnabled(!busy);
}

void LoginDialog::setStatusMessage(const QString& message, const bool error) {
    updateHintLabel(m_statusLabel, message, error);
}

void LoginDialog::togglePasswordVisible() {
    m_passwordVisible = !m_passwordVisible;
    m_passwordEdit->setEchoMode(m_passwordVisible ? QLineEdit::Normal : QLineEdit::Password);
    updatePasswordToggleIcon();
}

void LoginDialog::keyPressEvent(QKeyEvent* event) {
    if (event && event->key() == Qt::Key_Escape) {
        event->accept();
        return;
    }

    QDialog::keyPressEvent(event);
}

void LoginDialog::updatePasswordToggleIcon() const {
    if (!m_togglePasswordAction)
        return;

    m_togglePasswordAction->setIcon(QIcon(m_passwordVisible
        ? ":/res/icons/eye-off.svg"
        : ":/res/icons/eye.svg"));
    m_togglePasswordAction->setToolTip(m_passwordVisible ? tr("Hide password") : tr("Show password"));
}

void LoginDialog::updatePageToggleButton() {
    if (!m_pageToggleButton || !m_stackWidget || !m_loginPage || !m_networkPage)
        return;

    m_pageToggleButton->setText(m_stackWidget->currentWidget() == m_loginPage
        ? tr("Network Settings")
        : tr("< Login"));
}
