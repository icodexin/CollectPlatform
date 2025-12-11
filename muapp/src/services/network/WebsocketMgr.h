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
    Q_INVOKABLE void setHeartbeatParams(int interval, int timeout, int retries);
    Q_INVOKABLE void setReconnectParams(int maxAttempts, int baseDelay, int maxDelay, bool useJitter);

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
    QVariantMap m_heartbeatParams;
    QVariantMap m_reconnectParams;
    QString m_authToken;
};

#endif //WEBSOCKETMGR_H
