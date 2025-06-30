#ifndef MQTTPUBLISHER_H
#define MQTTPUBLISHER_H

#include <QMqttClient>
#include "services/DataQueue.h"

struct MessageParam {
    QString topic;
    QByteArray message;
    quint8 qos;
    bool retained;
};

class MqttPublisher : public QObject {
    Q_OBJECT

public:
    using MessageQueue = DataQueue<QPair<MessageParam, QByteArray> >;

    explicit MqttPublisher(QObject* parent = nullptr);

signals:
    void connected();

    void disconnected();

    void errorOccurred(QMqttClient::ClientError error);

    void messageSent(qint32 id);

public slots:
    Q_INVOKABLE void initialize(const QString& uid, const QString& username, const QString& password,
                                const QString& host = "127.0.0.1", int port = 1883);

    Q_INVOKABLE void publish(const QString& topic, const QByteArray& message, quint8 qos = 0, bool retained = false) const;

    Q_INVOKABLE void startPublishing(const QString& host = "127.0.0.1", int port = 1883) const;

    Q_INVOKABLE void stopPublishing() const;

private:
    QMqttClient* m_mqttClient;

    QString m_uid;
    QString m_statusTopic;
    QString m_dataTopicPrefix;
    QMqttUserProperties m_userProperties;
    QMqttPublishProperties m_publishStatusProperties;
    QMqttPublishProperties m_publishDataProperties;

    static QByteArray createOnlineMessage();

    static QByteArray createOfflineMessage();
};


#endif //MQTTPUBLISHER_H
