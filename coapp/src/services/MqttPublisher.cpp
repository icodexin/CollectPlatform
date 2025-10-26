#include "MqttPublisher.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QThread>

namespace {
    QByteArray createOnlineMessage() {
        return QJsonDocument(QJsonObject({
            {"timestamp", QDateTime::currentMSecsSinceEpoch()},
            {"status", "online"},
        })).toJson(QJsonDocument::Compact);
    }

    QByteArray createOfflineMessage() {
        return QJsonDocument(QJsonObject({
            {"timestamp", QDateTime::currentMSecsSinceEpoch()},
            {"status", "offline"},
        })).toJson(QJsonDocument::Compact);
    }
}

MqttPublisher::MqttPublisher(QObject* parent) : QObject(parent) {
}

MqttPublisher::~MqttPublisher() {
    disconnectFromBroker();
}

void MqttPublisher::init() {
    if (m_client)
        return;

    m_client = new QMqttClient(this);
    m_client->setProtocolVersion(QMqttClient::MQTT_5_0);
    m_client->setCleanSession(false);
    m_client->setKeepAlive(30);

    // 遗嘱消息
    m_client->setWillMessage(
        QJsonDocument(QJsonObject{{"status", "offline"}}).toJson(QJsonDocument::Compact)
    );
    m_client->setWillQoS(1);
    m_client->setWillRetain(true);

    connect(m_client, &QMqttClient::connected, this, [=] {
        m_client->publish(m_statusTopic, m_statusPublishProperties, createOnlineMessage(), 1, true);
        emit connected();
    });
    connect(m_client, &QMqttClient::disconnected, this, &MqttPublisher::disconnected);
    connect(m_client, &QMqttClient::errorChanged, this, [=](const QMqttClient::ClientError error) {
        if (error != QMqttClient::NoError) {
            emit errorOccurred(error);
        }
    });
    connect(m_client, &QMqttClient::messageSent, this, &MqttPublisher::messageSent);
}

void MqttPublisher::setHost(const QString& host) {
    if (host != m_host) {
        m_host = host;
    }
}

void MqttPublisher::setPort(const quint16 port) {
    if (port != m_port) {
        m_port = port;
    }
}

void MqttPublisher::setUid(const QString& uid) {
    if (uid != m_uid) {
        m_uid = uid;
        m_statusTopic = "student/" % m_uid % "/status";
        m_dataTopicPrefix = "student/" % m_uid % '/';

        // 自定义附加属性
        QMqttUserProperties userProperties;
        userProperties.emplace_back("uid", m_uid);

        // 遗嘱消息属性
        m_willProperties.setContentType("application/json");
        m_willProperties.setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator::UTF8Encoded);
        m_willProperties.setUserProperties(userProperties);

        // 在线状态消息属性
        m_statusPublishProperties.setContentType("application/json");
        m_statusPublishProperties.setPayloadFormatIndicator(QMqtt::PayloadFormatIndicator::UTF8Encoded);
        m_statusPublishProperties.setUserProperties(userProperties);

        // 数据消息属性
        m_dataPublishProperties.setContentType("application/msgpack");
        m_dataPublishProperties.setUserProperties(userProperties);
    }
}

void MqttPublisher::setUsername(const QString& username) {
    if (username != m_username) {
        m_username = username;
    }
}

void MqttPublisher::setPassword(const QString& password) {
    if (password != m_password) {
        m_password = password;
    }
}

void MqttPublisher::setBroker(const QString& host, const quint16 port) {
    setHost(host);
    setPort(port);
}

void MqttPublisher::setCredentials(const QString& uid, const QString& username, const QString& password) {
    setUid(uid);
    setUsername(username);
    setPassword(password);
}

void MqttPublisher::connectToBroker() {
    if (m_client && m_client->state() == QMqttClient::Disconnected) {
        m_client->setClientId(m_uid);
        m_client->setHostname(m_host);
        m_client->setPort(m_port);
        m_client->setUsername(m_username);
        m_client->setPassword(m_password);
        m_client->setWillTopic(m_statusTopic);
        m_client->setLastWillProperties(m_willProperties);
        m_client->connectToHost();
    }
}

void MqttPublisher::disconnectFromBroker() {
    if (!m_client)
        return;

    switch (m_client->state()) {
        case QMqttClient::Connected:
            m_client->publish(m_statusTopic, m_statusPublishProperties, createOfflineMessage(), 1, true);
            m_client->disconnectFromHost();
            break;
        case QMqttClient::Connecting:
            m_client->disconnectFromHost();
            break;
        default:
            break;
    }
}

int MqttPublisher::publish(const QString& topic, const QByteArray& message, const quint8 qos,
                           const bool retain) const {
    if (m_client && m_client->state() == QMqttClient::Connected) {
        return m_client->publish(m_dataTopicPrefix + topic, m_dataPublishProperties, message, qos, retain);
    }
    return -1;
}

QMqtt::ReasonCode MqttPublisher::reasonCode() const {
    if (m_client) {
        return m_client->serverConnectionProperties().reasonCode();
    }
    return QMqtt::ReasonCode::UnspecifiedError;
}

QString MqttPublisher::reason() const {
    if (m_client) {
        return m_client->serverConnectionProperties().reason();
    }
    return "Client not initialized";
}

MqttPublishService::MqttPublishService(QObject* parent) : QObject(parent) {
    m_publisher = new MqttPublisher(nullptr);
    m_workThread = new QThread(this);
    m_publisher->moveToThread(m_workThread);

    connect(m_publisher, &MqttPublisher::connected, this, [=] {
        setStatus(Running);
        emit started();
    });
    connect(m_publisher, &MqttPublisher::disconnected, this, [=] {
        setStatus(Stopped);
        emit stopped();
    });
    connect(m_publisher, &MqttPublisher::errorOccurred, this, &MqttPublishService::onClientErrorOccurred);
    connect(m_publisher, &MqttPublisher::messageSent, this, &MqttPublishService::messageSent);

    connect(m_workThread, &QThread::finished, m_publisher, &QObject::deleteLater);
    m_workThread->start();

    QMetaObject::invokeMethod(m_publisher, &MqttPublisher::init);
}

MqttPublishService::~MqttPublishService() {
    if (m_workThread && m_workThread->isRunning()) {
        m_workThread->quit();
        m_workThread->wait();
    }
}

void MqttPublishService::setHost(const QString& host) {
    QMetaObject::invokeMethod(m_publisher, &MqttPublisher::setHost, host);
}

void MqttPublishService::setPort(const quint16 port) {
    QMetaObject::invokeMethod(m_publisher, &MqttPublisher::setPort, port);
}

void MqttPublishService::setUid(const QString& uid) {
    QMetaObject::invokeMethod(m_publisher, &MqttPublisher::setUid, uid);
}

void MqttPublishService::setUsername(const QString& username) {
    QMetaObject::invokeMethod(m_publisher, &MqttPublisher::setUsername, username);
}

void MqttPublishService::setPassword(const QString& password) {
    QMetaObject::invokeMethod(m_publisher, &MqttPublisher::setPassword, password);
}

void MqttPublishService::setBroker(const QString& host, const quint16 port) {
    setHost(host);
    setPort(port);
}

void MqttPublishService::setCredentials(const QString& uid, const QString& username, const QString& password) {
    setUid(uid);
    setUsername(username);
    setPassword(password);
}

void MqttPublishService::start(const QString& host, const quint16 port, const QString& uid, const QString& username,
    const QString& password) {
    setBroker(host, port);
    setCredentials(uid, username, password);
    start();
}

void MqttPublishService::start() {
    setStatus(Starting);
    QMetaObject::invokeMethod(m_publisher, &MqttPublisher::connectToBroker);
}

void MqttPublishService::stop() {
    setStatus(Stopping);
    QMetaObject::invokeMethod(m_publisher, &MqttPublisher::disconnectFromBroker);
}

void MqttPublishService::publish(const QString& topic, const QByteArray& message, quint8 qos, bool retain) {
    QMetaObject::invokeMethod(
        m_publisher, &MqttPublisher::publish, Qt::QueuedConnection,
        topic, message, qos, retain
    );
}

void MqttPublishService::publishEEGData(const QByteArray& data) {
    return publish("eeg", data);
}

void MqttPublishService::publishBandData(const QByteArray& data) {
    return publish("wristband", data);
}

void MqttPublishService::setStatus(const Status status) {
    if (status != m_status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void MqttPublishService::onClientErrorOccurred(const QMqttClient::ClientError error) {
    if (error != QMqttClient::NoError) {
        QString msg;
        switch (error) {
            case QMqttClient::InvalidProtocolVersion:
                msg = tr("The broker does not accept a connection using the specified protocol version.");
                break;
            case QMqttClient::IdRejected:
                msg = tr("The client ID is malformed. This might be related to its length.");
                break;
            case QMqttClient::ServerUnavailable:
                msg = tr("The network connection has been established, "
                    "but the service is unavailable on the broker side");
                break;
            case QMqttClient::BadUsernameOrPassword:
                msg = tr("The data in the username or password is malformed.");
                break;
            case QMqttClient::NotAuthorized:
                msg = tr("The client is not authorized to connect.");
                break;
            case QMqttClient::TransportInvalid:
                msg = tr("The underlying transport caused an error. "
                    "For example, the connection might have been interrupted unexpectedly.");
                break;
            case QMqttClient::ProtocolViolation:
                msg = tr("The client encountered a protocol violation, and therefore closed the connection.");
                break;
            case QMqttClient::Mqtt5SpecificError:
                msg = tr("The error is related to MQTT protocol level 5. "
                          "More details may be help: Code: 0x%1, Reason: %2")
                      .arg(static_cast<int>(m_publisher->reasonCode()), 2, 16, QChar{'0'})
                      .arg(m_publisher->reason());
                break;
            case QMqttClient::UnknownError:
            default:
                msg = tr("An unknown error occurred.");
                break;
        }
        emit errorOccurred(error, msg);
    }
}
