#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QtNetwork/QAbstractSocket>
#include <QtQml/qqml.h>

class QTimer;
class QWebSocket;

class WebsocketClient final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(HttpHeaders upgradeHeaders READ upgradeHeaders WRITE setUpgradeHeaders NOTIFY upgradeHeadersChanged)
    Q_PROPERTY(QStringList requestedSubprotocols READ requestedSubprotocols WRITE setRequestedSubprotocols NOTIFY requestedSubprotocolsChanged)
    Q_PROPERTY(QString negotiatedSubprotocol READ negotiatedSubprotocol NOTIFY negotiatedSubprotocolChanged)
    Q_PROPERTY(int heartbeatInterval READ heartbeatInterval WRITE setHeartbeatInterval NOTIFY heartbeatIntervalChanged)
    Q_PROPERTY(int heartbeatTimeout READ heartbeatTimeout WRITE setHeartbeatTimeout NOTIFY heartbeatTimeoutChanged)
    Q_PROPERTY(int heartbeatRetries READ heartbeatRetries WRITE setHeartbeatRetries NOTIFY heartbeatRetriesChanged)
    Q_PROPERTY(int reconnectMaxAttempts READ reconnectMaxAttempts WRITE setReconnectMaxAttempts NOTIFY reconnectMaxAttemptsChanged)
    Q_PROPERTY(int reconnectBaseDelay READ reconnectBaseDelay WRITE setReconnectBaseDelay NOTIFY reconnectBaseDelayChanged)
    Q_PROPERTY(int reconnectMaxDelay READ reconnectMaxDelay WRITE setReconnectMaxDelay NOTIFY reconnectMaxDelayChanged)
    Q_PROPERTY(bool useJitter READ useJitter WRITE setUseJitter NOTIFY useJitterChanged)
    Q_PROPERTY(int reconnectAttempts READ reconnectAttempts NOTIFY reconnectAttemptsChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorOccurred)

public:
    using HttpHeaders = QList<std::pair<QByteArray, QByteArray>>;
    static constexpr int defaultHeartbeatInterval = 5000; // ms
    static constexpr int defaultHeartbeatTimeout = 500;   // ms
    static constexpr int defaultHeartbeatRetries = 2;
    static constexpr int defaultReconnectMaxAttempts = 5;
    static constexpr int defaultReconnectBaseDelay = 500;  // ms
    static constexpr int defaultReconnectMaxDelay = 30000; // ms
    static constexpr bool defaultUseJitter = true;

    enum Status {
        Connecting   = 0,
        Open         = 1,
        Closing      = 2,
        Closed       = 3,
        Reconnecting = 4
    };
    Q_ENUM(Status)

    explicit WebsocketClient(QObject* parent = nullptr);
    ~WebsocketClient() override;

    QUrl url() const;
    void setUrl(const QUrl& url);

    /// 握手时升级Websocket协议的HTTP附加头
    HttpHeaders upgradeHeaders() const;
    void setUpgradeHeaders(const HttpHeaders& headers);

    /// 请求的子协议列表
    QStringList requestedSubprotocols() const;
    void setRequestedSubprotocol(const QString& protocol);
    void setRequestedSubprotocols(const QStringList& protocols);

    /// 与服务器协商后的子协议
    QString negotiatedSubprotocol() const;

    /// 心跳检测间隔, 等于0表示不启用心跳
    int heartbeatInterval() const;
    void setHeartbeatInterval(int msec);
    /// 心跳检测超时阈值
    int heartbeatTimeout() const;
    void setHeartbeatTimeout(int msec);
    /// 心跳检测重试次数
    int heartbeatRetries() const;
    void setHeartbeatRetries(int count);
    /// 心跳检测参数设置
    Q_INVOKABLE void setHeartbeat(int interval, int timeout, int retries);

    /// 自动重连最大尝试次数, 等于0表示不启用自动重连
    int reconnectMaxAttempts() const;
    void setReconnectMaxAttempts(int count);
    /// 自动重连指数退避基础时延
    int reconnectBaseDelay() const;
    void setReconnectBaseDelay(int msec);
    /// 自动重连最大时延
    int reconnectMaxDelay() const;
    void setReconnectMaxDelay(int msec);
    /// 自动重连随机扰动
    bool useJitter() const;
    void setUseJitter(bool jitter);
    /// 自动重连参数设置
    Q_INVOKABLE void setReconnect(int maxAttempts, int baseDelay, int maxDelay, bool useJitter);
    /// 已尝试重连次数
    int reconnectAttempts() const;

    Status status() const;
    QString errorString() const;

    Q_INVOKABLE void open();
    Q_INVOKABLE void close();

    Q_INVOKABLE qint64 sendText(const QString& text);
    Q_INVOKABLE qint64 sendBinary(const QByteArray& data);
    Q_INVOKABLE qint64 sendJson(const QJsonObject& json);
    Q_INVOKABLE void ping(const QByteArray& payload = {});

signals:
    void urlChanged(const QUrl& url);
    void upgradeHeadersChanged(const WebsocketClient::HttpHeaders& headers);
    void requestedSubprotocolsChanged(const QStringList& protocols);
    void negotiatedSubprotocolChanged(const QString& protocol);

    void heartbeatIntervalChanged(int interval);
    void heartbeatTimeoutChanged(int count);
    void heartbeatRetriesChanged(int count);

    void reconnectMaxAttemptsChanged(int count);
    void reconnectBaseDelayChanged(int msec);
    void reconnectMaxDelayChanged(int msec);
    void useJitterChanged(bool jitter);
    void reconnectAttemptsChanged(int);

    void textReceived(const QString& text);
    void binaryReceived(const QByteArray& data);
    void pongReceived(quint64 elapsed, const QByteArray& payload);
    void errorOccurred(QAbstractSocket::SocketError err, const QString& errString);

    void statusChanged(Status status);

private slots:
    void onError(QAbstractSocket::SocketError error);
    void onStateChanged(QAbstractSocket::SocketState state);
    void onPong(quint64 elapsed, const QByteArray& payload);
    void onHeartbeat();
    void onReconnect();

private:
    // 设置底层WebSocket对象
    void setSocket(QWebSocket* socket);
    // 设置连接状态
    void setStatus(Status status);
    // 打开连接
    void openSocket();
    // 心跳检测
    void startHeartbeat();
    void stopHeartbeat();
    // 自动重连
    void scheduleReconnect();
    void stopReconnect() const;
    int nextReconnectDelay() const;
    void clearReconnectAttempts();
    void addReconnectAttempts();

private:
    QScopedPointer<QWebSocket> m_socket;
    QTimer* m_heartbeatTimer = nullptr;
    QTimer* m_reconnectTimer = nullptr;

    QUrl m_url;
    HttpHeaders m_upgradeHeaders;
    QStringList m_requestedSubprotocols;
    QString m_negotiatedSubprotocol;
    QString m_errString;

    int m_heartbeatInterval = defaultHeartbeatInterval; // 心跳检测间隔, 等于0时不启用心跳检测
    int m_heartbeatTimeout = defaultHeartbeatTimeout;   // 心跳检测超时阈值
    int m_heartbeatRetries = defaultHeartbeatRetries;   // 心跳检测重试次数
    // 心跳检测总等待时间 = 超时阈值 + 重试次数 * 心跳间隔
    int m_heartbeatPatience = defaultHeartbeatTimeout + defaultHeartbeatRetries * defaultHeartbeatInterval;
    qint64 m_lastPingTimestamp = 0; // 发送最后一个ping的时间戳
    qint64 m_lastPongTimestamp = 0; // 收到最后一个pong的时间戳

    int m_reconnectAttempts = 0;                              // 已尝试重连次数
    int m_reconnectBaseDelay = defaultReconnectBaseDelay;     // 指数退避基础时延
    int m_reconnectMaxDelay = defaultReconnectMaxDelay;       // 重连最大时延
    int m_reconnectMaxAttempts = defaultReconnectMaxAttempts; // 最大重连次数, 等于0时不启用自动重连
    bool m_useJitter = defaultUseJitter;                      // 随机扰动
    bool m_manualCloseFlag = false;                           // 主动关闭标志

    Status m_status = Closed;
};

#endif //WEBSOCKETCLIENT_H
