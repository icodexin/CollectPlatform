#ifndef MQTTPUBLISHVIEW_H
#define MQTTPUBLISHVIEW_H

#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QThread>
#include "components/BarCard.h"
#include "components/LogEdit.h"
#include "components/StatusIndicator.h"
#include "services/MqttPublisher.h"


class MqttPublishView : public BarCard {
    Q_OBJECT

public:
    explicit MqttPublishView(const QString& uid, const QString& mqttUserName, const QString& mqttPassword,
                             const QString& host = "127.0.0.1", int port = 1883, QWidget* parent = nullptr);

    ~MqttPublishView() override;

    void log(const QString& msg, LogEdit::LogLevel level = LogEdit::LogLevel::Info) const;

public slots:
    void publish(const QString& topic, const QByteArray& message, quint8 qos = 0, bool retained = false) const;

    void stopPublishing() const;

private slots:
    void setUIConnecting() const;

    void setUIConnected() const;

    void setUIDisconnecting() const;

    void setUIDisconnected() const;

    void onMqttConnected() const;

    void onMqttDisconnected() const;

    void onMqttErrorOccurred(QMqttClient::ClientError error) const;

private:
    LogEdit* ui_logEdit;
    QLineEdit* ui_hostEdit;
    QSpinBox* ui_portEdit;
    QPushButton* ui_clearBtn;
    QPushButton* ui_publishBtn;
    StatusIndicator* ui_indicator;

    MqttPublisher* m_mqttPublisher;
    QThread m_workerThread;

    void initUI(const QString& host, int port);

    void initConnection();
};


#endif //MQTTPUBLISHVIEW_H
