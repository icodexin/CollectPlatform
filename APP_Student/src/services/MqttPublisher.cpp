#include "MqttPublisher.h"
#include <QJsonDocument>
#include <QJsonObject>

MqttPublisher::MqttPublisher(QObject* parent): QObject(parent) {
}

void MqttPublisher::initialize(const QString& uid, const QString& username, const QString& password,
                               const QString& host, int port) {
    m_statusTopic = "student/" + uid + "/status";
    m_dataTopicPrefix = "student/" + uid + '/';

    m_mqttClient = new QMqttClient(this);
    m_mqttClient->setProtocolVersion(QMqttClient::MQTT_5_0);
    m_mqttClient->setCleanSession(false);
    m_mqttClient->setKeepAlive(30);
    m_mqttClient->setClientId("student." + uid);
    m_mqttClient->setUsername(username);
    m_mqttClient->setPassword(password);

    m_mqttClient->setWillTopic(m_statusTopic);
    m_mqttClient->setWillMessage(
        QJsonDocument(QJsonObject{{"status", "offline"}}).toJson(QJsonDocument::Compact)
    );
    m_mqttClient->setWillQoS(1);
    m_mqttClient->setWillRetain(true);

    m_userProperties.emplace_back("uid", uid);

    QMqttLastWillProperties willProperties;
    willProperties.setContentType("application/json");
    willProperties.setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator::UTF8Encoded);
    willProperties.setUserProperties(m_userProperties);
    m_mqttClient->setLastWillProperties(willProperties);

    m_publishStatusProperties.setContentType("application/json");
    m_publishStatusProperties.setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator::UTF8Encoded);
    m_publishStatusProperties.setUserProperties(m_userProperties);

    m_publishDataProperties.setContentType("application/msgpack");
    m_publishDataProperties.setUserProperties(m_userProperties);

    connect(m_mqttClient, &QMqttClient::connected, [=] {
        const QByteArray msg = createOnlineMessage();
        m_mqttClient->publish(m_statusTopic, m_publishStatusProperties, msg, 1, true);
        emit connected();
    });

    connect(m_mqttClient, &QMqttClient::disconnected, [=] {
        emit disconnected();
    });

    connect(m_mqttClient, &QMqttClient::errorChanged, [=] {
        if (m_mqttClient->error() != QMqttClient::NoError) {
            emit errorOccurred(m_mqttClient->error());
        }
    });

    connect(m_mqttClient, &QMqttClient::messageSent, [=](const qint32 id) {
        emit messageSent(id);
    });
}

void MqttPublisher::startPublishing(const QString& host, int port) const {
    m_mqttClient->setHostname(host);
    m_mqttClient->setPort(port);
    m_mqttClient->connectToHost();
}

void MqttPublisher::stopPublishing() const {
    switch (m_mqttClient->state()) {
        case QMqttClient::Connected: {
            const QByteArray msg = createOfflineMessage();
            m_mqttClient->publish(m_statusTopic, m_publishStatusProperties, msg, 1, true);
            m_mqttClient->disconnectFromHost();
            break;
        }
        case QMqttClient::Connecting: {
            m_mqttClient->disconnectFromHost();
            break;
        }
        default:
            break;
    }
}

void MqttPublisher::publish(const QString& topic, const QByteArray& message, const quint8 qos,
                            const bool retained) const {
    if (m_mqttClient->state() == QMqttClient::Connected) {
        m_mqttClient->publish(
            m_dataTopicPrefix + topic, m_publishDataProperties, message, qos, retained);
    }
}

QByteArray MqttPublisher::createOnlineMessage() {
    return QJsonDocument(QJsonObject({
        {"timestamp", QDateTime::currentMSecsSinceEpoch()},
        {"status", "online"},
    })).toJson(QJsonDocument::Compact);
}

QByteArray MqttPublisher::createOfflineMessage() {
    return QJsonDocument(QJsonObject({
        {"timestamp", QDateTime::currentMSecsSinceEpoch()},
        {"status", "offline"},
    })).toJson(QJsonDocument::Compact);
}
