#ifndef USERINFODIALOG_H
#define USERINFODIALOG_H

#include <QtCore/QJsonObject>
#include <QtWidgets/QDialog>

class QLabel;
class QPushButton;
class QPlainTextEdit;
class QShowEvent;

class UserInfoDialog final : public QDialog {
    Q_OBJECT

public:
    explicit UserInfoDialog(QWidget* parent = nullptr);

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onCurrentUserFetched(const QJsonObject& userInfo);
    void onCurrentUserFetchFailed(int statusCode, const QString& message);
    void onLogoutClicked();

private:
    void initUI();
    void loadUserInfo();
    void setFieldValue(QLabel* label, const QString& value);

private:
    QLabel* m_statusLabel = nullptr;
    QLabel* m_nameValueLabel = nullptr;
    QLabel* m_idValueLabel = nullptr;
    QLabel* m_typeValueLabel = nullptr;
    QLabel* m_activeValueLabel = nullptr;
    QPlainTextEdit* m_profileView = nullptr;
    QPushButton* m_logoutButton = nullptr;
    bool m_hasLoadedOnce = false;
};

#endif //USERINFODIALOG_H
