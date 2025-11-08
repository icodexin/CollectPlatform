#ifndef MQTTSETTINGPANEL_H
#define MQTTSETTINGPANEL_H

#include <QGroupBox>

class IPv4Edit;
class QPushButton;
class QLineEdit;
class QSpinBox;

class MqttSettingPanel final : public QGroupBox {
    Q_OBJECT

public:
    explicit MqttSettingPanel(QWidget* parent = nullptr);
    QString address() const;
    int port() const;
    QString id() const;
    QString username() const;
    QString password() const;

signals:
    void requestConnect(const QString& address, int port, const QString& id,
                        const QString& username, const QString& password);
    void requestDisconnect();

public slots:
    void handleConnected() const;
    void handleDisconnected() const;

private slots:
    void onConnectBtnClicked();

private:
    IPv4Edit* m_addressEdit = nullptr;
    QSpinBox* m_portSpinBox = nullptr;
    QLineEdit* m_idEdit = nullptr;
    QLineEdit* m_usernameEdit = nullptr;
    QLineEdit* m_passwordEdit = nullptr;
    QPushButton* m_connectBtn = nullptr;
};

#endif // MQTTSETTINGPANEL_H
