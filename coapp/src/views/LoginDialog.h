#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QtGui/QKeyEvent>
#include <QtWidgets/QDialog>

class QAction;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;
class QWidget;

class LoginDialog final : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget* parent = nullptr);

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onSaveNetworkClicked();
    void togglePage();
    void onLoginSucceeded();
    void onLoginFailed(const QString& message);
    void togglePasswordVisible();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void initUI();
    void initLoginPage();
    void initNetworkPage();
    void setBusy(bool busy);
    void setStatusMessage(const QString& message, bool error = false);
    void updatePasswordToggleIcon() const;
    void updatePageToggleButton();

private:
    QStackedWidget* m_stackWidget = nullptr;
    QWidget* m_loginPage = nullptr;
    QWidget* m_networkPage = nullptr;
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QLineEdit* m_serverAddressEdit = nullptr;
    QLabel* m_statusLabel = nullptr;
    QLabel* m_networkStatusLabel = nullptr;
    QPushButton* m_loginButton = nullptr;
    QPushButton* m_registerButton = nullptr;
    QPushButton* m_pageToggleButton = nullptr;
    QPushButton* m_saveNetworkButton = nullptr;
    QAction* m_togglePasswordAction = nullptr;
    bool m_busy = false;
    bool m_passwordVisible = false;
};

#endif //LOGINDIALOG_H
