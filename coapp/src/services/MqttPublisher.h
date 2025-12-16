#ifndef MQTTPUBLISHER_H
#define MQTTPUBLISHER_H

#include <QObject>
#include <QMqttClient>

class MqttPublisher final : public QObject {
    Q_OBJECT

public:
    explicit MqttPublisher(QObject* parent = nullptr);
    ~MqttPublisher() override;

signals:
    void connected();
    void disconnected();
    void errorOccurred(QMqttClient::ClientError error);
    void messageSent(qint32 id);

public slots:
    Q_INVOKABLE void init();
    Q_INVOKABLE void setHost(const QString& host);
    Q_INVOKABLE void setPort(quint16 port);
    Q_INVOKABLE void setUid(const QString& uid);
    Q_INVOKABLE void setUsername(const QString& username);
    Q_INVOKABLE void setPassword(const QString& password);
    Q_INVOKABLE void setBroker(const QString& host, quint16 port);
    Q_INVOKABLE void setCredentials(const QString& uid, const QString& username, const QString& password);
    Q_INVOKABLE void connectToBroker();
    Q_INVOKABLE void disconnectFromBroker();
    Q_INVOKABLE int publish(const QString& topic, const QByteArray& message, quint8 qos = 0,
                            bool retain = false) const;
    Q_INVOKABLE QMqtt::ReasonCode reasonCode() const;
    Q_INVOKABLE QString reason() const;

private:
    QMqttClient* m_client = nullptr;

    QString m_host = "localhost"; // MQTT代理主机
    quint16 m_port = 1883;        // MQTT代理端口
    QString m_uid;                // 用户ID
    QString m_username;           // 用户名
    QString m_password;           // 密码
    QString m_statusTopic;        // 在线状态主题
    QString m_dataTopicPrefix;    // 数据主题前缀

    QMqttLastWillProperties m_willProperties;
    QMqttPublishProperties m_statusPublishProperties;
    QMqttPublishProperties m_dataPublishProperties;
};

class MqttPublishService final : public QObject {
    Q_OBJECT

public:
    enum Status {
        Stopped = 0,
        Starting,
        Running,
        Stopping
    };

    Q_ENUM(Status)

    explicit MqttPublishService(QObject* parent = nullptr);
    ~MqttPublishService() override;

signals:
    void started();
    void stopped();
    void statusChanged(Status status);
    void errorOccurred(QMqttClient::ClientError error, const QString& errorString);
    void messageSent(qint32 id);

public slots:
    void setHost(const QString& host);
    void setPort(quint16 port);
    void setUid(const QString& uid);
    void setUsername(const QString& username);
    void setPassword(const QString& password);
    void setBroker(const QString& host, quint16 port);
    void setCredentials(const QString& uid, const QString& username, const QString& password);

    void start(const QString& host, quint16 port, const QString& uid, const QString& username, const QString& password);
    void start();
    void stop();

    void publish(const QString& topic, const QByteArray& message, quint8 qos, bool retain);
    void publish(const QString& topic, const QByteArray& message /* quint8 qos = 0, bool retain = false */);

private slots:
    void setStatus(Status status);
    void onClientErrorOccurred(QMqttClient::ClientError error);

private:
    MqttPublisher* m_publisher = nullptr;
    QThread* m_workThread = nullptr;
    Status m_status = Stopped;
};

#endif //MQTTPUBLISHER_H
