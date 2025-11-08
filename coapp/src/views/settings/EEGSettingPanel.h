#ifndef EEGSETTINGPANEL_H
#define EEGSETTINGPANEL_H

#include <QGroupBox>

class IPv4Edit;
class QSpinBox;
class QPushButton;

class EEGSettingPanel final : public QGroupBox {
    Q_OBJECT

public:
    explicit EEGSettingPanel(QWidget* parent = nullptr);
    QString address() const;
    int port() const;

signals:
    void requestConnect(const QString& address, int port);
    void requestDisconnect();

public slots:
    void handleConnected() const;
    void handleDisconnected() const;

private slots:
    void onConnectBtnClicked();

private:
    IPv4Edit* m_addressEdit = nullptr;
    QSpinBox* m_portSpinBox = nullptr;
    QPushButton* m_connectBtn = nullptr;
};

#endif // EEGSETTINGPANEL_H
