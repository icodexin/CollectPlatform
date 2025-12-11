#ifndef WEBSOCKETMGR_H
#define WEBSOCKETMGR_H

#include <QtCore/QMap>
#include "singleton.h"
#include "WebsocketClient.h"

#define MuWebsocketMgr WebsocketMgr::instance()

class WebsocketMgr final : public QObject, public Singleton<WebsocketMgr> {
    Q_OBJECT
    DECLARE_SINGLETON(WebsocketMgr)
    Q_DISABLE_COPY_MOVE(WebsocketMgr)

public:
    struct HeartbeatParam {
        int interval = WebsocketClient::defaultHeartbeatInterval;
        int timeout = WebsocketClient::defaultHeartbeatTimeout;
        int retries = WebsocketClient::defaultHeartbeatRetries;
    };

    struct ReconnectParam {
        int maxAttempts = WebsocketClient::defaultReconnectMaxAttempts;
        int baseDelay = WebsocketClient::defaultReconnectBaseDelay;
        int maxDelay = WebsocketClient::defaultReconnectMaxDelay;
        bool useJitter = WebsocketClient::defaultUseJitter;
        int baseNumber = WebsocketClient::defaultReconnectBaseNumber;
    };

    /// 设置全局心跳参数
    Q_INVOKABLE void setHeartbeatParam(const HeartbeatParam& param);
    /// 设置指定连接的心跳参数
    Q_INVOKABLE void setHeartbeatParam(const QString& key, const HeartbeatParam& param);
    /// 设置全局重连参数
    Q_INVOKABLE void setReconnectParam(const ReconnectParam& param);
    /// 设置指定连接的重连参数
    Q_INVOKABLE void setReconnectParam(const QString& key, const ReconnectParam& param);

    Q_INVOKABLE bool createConnection(const QString& key, const QUrl& url);
    Q_INVOKABLE bool removeConnection(const QString& key);
    Q_INVOKABLE void removeAllConnections();
    Q_INVOKABLE bool hasConnection(const QString& key) const;

    Q_INVOKABLE void open(const QString& key) const;
    Q_INVOKABLE void close(const QString& key) const;

    Q_INVOKABLE void openAll() const;
    Q_INVOKABLE void closeAll();

    Q_INVOKABLE void sendText(const QString& key, const QString& text) const;
    Q_INVOKABLE void sendJson(const QString& key, const QJsonObject& json) const;
    Q_INVOKABLE void sendBinary(const QString& key, const QByteArray& data) const;

    QPointer<WebsocketClient> client(const QString& key) const;

    Q_INVOKABLE QString authToken() const;
    Q_INVOKABLE void setAuthToken(const QString& token);

signals:
    void textReceived(const QString& key, const QString& text);
    void binaryReceived(const QString& key, const QByteArray& data);
    void statsChanged(const QString& key, const WebsocketClient::Status& stats);
    void errorOccurred(const QString& key, const QAbstractSocket::SocketError& error, const QString& errorString);

private:
    explicit WebsocketMgr(QObject* parent = nullptr);
    ~WebsocketMgr() override;

    // move client to worker thread
    void setClient(WebsocketClient* client);
    void attachSignals(const QString& key, const WebsocketClient* client);

    static void openClient(WebsocketClient* client);
    static void closeClient(WebsocketClient* client);
    static void removeClient(WebsocketClient* client);

private:
    QThread* m_workerThread = nullptr;
    QHash<QString, WebsocketClient*> m_clients;
    QHash<QString, HeartbeatParam> m_clientHeartbeatParams;
    QHash<QString, ReconnectParam> m_clientReconnectParams;
    HeartbeatParam m_globalHeartbeatParam;
    ReconnectParam m_globalReconnectParam;
    QString m_authToken;
};

#endif //WEBSOCKETMGR_H
