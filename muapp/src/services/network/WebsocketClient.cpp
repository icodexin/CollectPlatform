#include "WebsocketClient.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QLoggingCategory>
#include <QtCore/QRandomGenerator>
#include <QtCore/QTimer>
#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketHandshakeOptions>

namespace {
    constexpr auto kHeartbeatPayload = "HEARTBEAT";
}

Q_LOGGING_CATEGORY(wsClient, "Services.Network.WebsocketClient")

WebsocketClient::WebsocketClient(QObject* parent) : QObject(parent) {
}

WebsocketClient::~WebsocketClient() {
    stopReconnect();
    stopHeartbeat();
    if (m_socket) {
        disconnect(m_socket.get(), nullptr, this, nullptr);
    }
}

void WebsocketClient::init() {
    setSocket(new QWebSocket);

    m_heartbeatTimer = new QTimer(this);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &WebsocketClient::onHeartbeat);

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &WebsocketClient::onReconnect);

    connect(this, &WebsocketClient::heartbeatIntervalChanged, this, [this](const int interval) {
        if (m_status == Open) {
            if (interval > 0)
                startHeartbeat();
            else
                stopHeartbeat();
        }
    });
    connect(this, &WebsocketClient::reconnectMaxAttemptsChanged, this, [this](const int count) {
        if (count <= 0) {
            stopReconnect();
        }
    });
}

QUrl WebsocketClient::url() const {
    return m_url;
}

void WebsocketClient::setUrl(const QUrl& url) {
    if (m_url == url)
        return;
    if (m_status != Closed) {
        qCWarning(wsClient) << QString("[%1] Cannot change URL while the socket is open.").arg(m_url.toString());
        return;
    }
    m_url = url;
    emit urlChanged(url);
}

WebsocketClient::HttpHeaders WebsocketClient::upgradeHeaders() const {
    return m_upgradeHeaders;
}

void WebsocketClient::setUpgradeHeaders(const HttpHeaders& headers) {
    if (m_upgradeHeaders == headers)
        return;
    m_upgradeHeaders = headers;
    emit upgradeHeadersChanged(headers);
}

QStringList WebsocketClient::requestedSubprotocols() const {
    return m_requestedSubprotocols;
}

void WebsocketClient::setRequestedSubprotocol(const QString& protocol) {
    setRequestedSubprotocols({protocol});
}

void WebsocketClient::setRequestedSubprotocols(const QStringList& protocols) {
    if (m_requestedSubprotocols == protocols)
        return;
    m_requestedSubprotocols = protocols;
    emit requestedSubprotocolsChanged(protocols);
}

QString WebsocketClient::negotiatedSubprotocol() const {
    return m_negotiatedSubprotocol;
}

int WebsocketClient::heartbeatInterval() const {
    return m_heartbeatInterval;
}

void WebsocketClient::setHeartbeatInterval(const int msec) {
    if (m_heartbeatInterval == msec)
        return;
    m_heartbeatInterval = msec > 0 ? msec : 0;
    m_heartbeatPatience = m_heartbeatTimeout + m_heartbeatRetries * m_heartbeatInterval;
    emit heartbeatIntervalChanged(m_heartbeatInterval);
}

int WebsocketClient::heartbeatTimeout() const {
    return m_heartbeatTimeout;
}

void WebsocketClient::setHeartbeatTimeout(const int msec) {
    if (m_heartbeatTimeout == msec)
        return;
    m_heartbeatTimeout = msec > 0 ? msec : 0;
    m_heartbeatPatience = m_heartbeatTimeout + m_heartbeatRetries * m_heartbeatInterval;
    emit heartbeatTimeoutChanged(m_heartbeatTimeout);
}

int WebsocketClient::heartbeatRetries() const {
    return m_heartbeatRetries;
}

void WebsocketClient::setHeartbeatRetries(const int count) {
    if (m_heartbeatRetries == count)
        return;
    m_heartbeatRetries = count > 0 ? count : 0;
    m_heartbeatPatience = m_heartbeatTimeout + m_heartbeatRetries * m_heartbeatInterval;
    emit heartbeatRetriesChanged(m_heartbeatRetries);
}

void WebsocketClient::setHeartbeat(const int interval, const int timeout, const int retries) {
    setHeartbeatTimeout(timeout);
    setHeartbeatRetries(retries);
    setHeartbeatInterval(interval);
}

bool WebsocketClient::useReconnect() const {
    return m_useReconnect;
}

void WebsocketClient::setUseReconnect(bool use) {
    if (m_useReconnect == use)
        return;
    m_useReconnect = use;
    emit useReconnectChanged(use);
}

int WebsocketClient::reconnectMaxAttempts() const {
    return m_reconnectMaxAttempts;
}

void WebsocketClient::setReconnectMaxAttempts(const int count) {
    if (m_reconnectMaxAttempts == count)
        return;
    m_reconnectMaxAttempts = count > 0 ? count : 0;
    emit reconnectMaxAttemptsChanged(m_reconnectMaxAttempts);
}

int WebsocketClient::reconnectBaseNumber() const {
    return m_reconnectBaseNumber;
}

void WebsocketClient::setReconnectBaseNumber(const int base) {
    if (m_reconnectBaseNumber == base)
        return;
    m_reconnectBaseNumber = base > 0 ? base : 1;
    emit reconnectBaseNumberChanged(m_reconnectBaseNumber);
}

int WebsocketClient::reconnectBaseDelay() const {
    return m_reconnectBaseDelay;
}

void WebsocketClient::setReconnectBaseDelay(const int msec) {
    if (m_reconnectBaseDelay == msec)
        return;
    m_reconnectBaseDelay = msec > 0 ? msec : 0;
    emit reconnectBaseDelayChanged(m_reconnectBaseDelay);
}

int WebsocketClient::reconnectMaxDelay() const {
    return m_reconnectMaxDelay;
}

void WebsocketClient::setReconnectMaxDelay(const int msec) {
    if (m_reconnectMaxDelay == msec)
        return;
    m_reconnectMaxDelay = msec > 0 ? msec : 0;
    emit reconnectMaxDelayChanged(m_reconnectMaxDelay);
}

bool WebsocketClient::useJitter() const {
    return m_useJitter;
}

void WebsocketClient::setUseJitter(const bool jitter) {
    if (m_useJitter == jitter)
        return;
    m_useJitter = jitter;
    emit useJitterChanged(jitter);
}

void WebsocketClient::setReconnect(const int maxAttempts, const int baseDelay, const int maxDelay,
                                   const bool useJitter, const int baseNumber) {
    setReconnectBaseNumber(baseNumber);
    setReconnectBaseDelay(baseDelay);
    setReconnectMaxDelay(maxDelay);
    setUseJitter(useJitter);
    setReconnectMaxAttempts(maxAttempts);
}

int WebsocketClient::reconnectAttempts() const {
    return m_reconnectAttempts;
}

WebsocketClient::Status WebsocketClient::status() const {
    return m_status;
}

QString WebsocketClient::errorString() const {
    return m_errString;
}

void WebsocketClient::open() {
    if (m_status == Closed || m_status == Reconnecting) {
        stopReconnect();
        m_manualCloseFlag = false;
        clearReconnectAttempts();
        setStatus(Connecting);
        openSocket();
        return;
    }
    qCDebug(wsClient) << QString("[%1] Websocket is already open or connecting.").arg(m_url.toString());
}

void WebsocketClient::close() {
    if (m_status != Closed) {
        stopReconnect();
        m_manualCloseFlag = true;
        if (m_socket) {
            m_socket->close();
        }
        return;
    }
    qCDebug(wsClient) << QString("[%1] Websocket is already closed.").arg(m_url.toString());
}

qint64 WebsocketClient::sendText(const QString& text) {
    if (!canSendMessages()) {
        enqueueTextMessage(text);
        qCDebug(wsClient) << QString("[%1] Socket not open, queued text message.").arg(m_url.toString());
        return text.size();
    }
    return m_socket->sendTextMessage(text);
}

qint64 WebsocketClient::sendBinary(const QByteArray& data) {
    if (!canSendMessages()) {
        enqueueBinaryMessage(data);
        qCDebug(wsClient) << QString("[%1] Socket not open, queued binary message.").arg(m_url.toString());
        return data.size();
    }
    return m_socket->sendBinaryMessage(data);
}

qint64 WebsocketClient::sendJson(const QJsonObject& json) {
    return sendText(QString::fromUtf8(QJsonDocument(json).toJson(QJsonDocument::Compact)));
}

void WebsocketClient::ping(const QByteArray& payload) {
    if (m_status != Open || !m_socket) {
        m_errString = tr("Ping Messages can only be sent when the socket is open.");
        emit errorOccurred(QAbstractSocket::SocketError::OperationError, m_errString);
        return;
    }
    m_socket->ping(payload);
    m_lastPingTimestamp = QDateTime::currentMSecsSinceEpoch();
}

void WebsocketClient::onError(const QAbstractSocket::SocketError error) {
    m_errString = m_socket ? m_socket->errorString() : QString();
    emit errorOccurred(error, m_errString);
}

void WebsocketClient::onStateChanged(const QAbstractSocket::SocketState state) {
    switch (state) {
        case QAbstractSocket::ConnectingState:
        case QAbstractSocket::BoundState:
        case QAbstractSocket::HostLookupState:
            // 外部通过 open()/onReconnect() 区分为 Connecting/Reconnecting
            break;
        case QAbstractSocket::UnconnectedState: {
            if (!m_manualCloseFlag && m_useReconnect)
                scheduleReconnect();
            else
                setStatus(Closed);
            break;
        }
        case QAbstractSocket::ConnectedState: {
            setStatus(Open);
            break;
        }
        case QAbstractSocket::ClosingState: {
            setStatus(Closing);
            break;
        }
        default:
            break;
    }
}

void WebsocketClient::onPong(const quint64 elapsed, const QByteArray& payload) {
    if (payload == kHeartbeatPayload) {
        qCDebug(wsClient) << QString("[%1] Heartbeat pong received after %2 ms.")
                             .arg(url().toString())
                             .arg(elapsed);
        m_lastPongTimestamp = QDateTime::currentMSecsSinceEpoch();
    }
    emit pongReceived(elapsed, payload);
}

void WebsocketClient::onHeartbeat() {
    if (m_status != Open || !m_socket) {
        return;
    }

    const qint64 now = QDateTime::currentMSecsSinceEpoch();
    const qint64 pongElapsed = now - m_lastPongTimestamp;

    if (m_lastPongTimestamp > 0 && pongElapsed > m_heartbeatPatience) {
        qCWarning(wsClient) << QString("[%1] Heartbeat timeout after %1 ms. Aborting.")
                               .arg(url().toString())
                               .arg(pongElapsed);
        m_socket->abort();
        return;
    }

    m_lastPingTimestamp = now;
    ping(kHeartbeatPayload);
}

void WebsocketClient::onReconnect() {
    if (m_status == Reconnecting && !m_manualCloseFlag)
        openSocket();
}

void WebsocketClient::setSocket(QWebSocket* socket) {
    m_socket.reset(socket);
    if (m_socket) {
        m_socket->setParent(nullptr);
        connect(m_socket.get(), &QWebSocket::textMessageReceived, this, &WebsocketClient::textReceived);
        connect(m_socket.get(), &QWebSocket::binaryMessageReceived, this, &WebsocketClient::binaryReceived);
        connect(m_socket.get(), &QWebSocket::pong, this, &WebsocketClient::onPong);
        connect(m_socket.get(), &QWebSocket::errorOccurred, this, &WebsocketClient::onError);
        connect(m_socket.get(), &QWebSocket::stateChanged, this, &WebsocketClient::onStateChanged);
    }
}

void WebsocketClient::setStatus(const Status status) {
    if (m_status == status)
        return;

    const Status old = m_status;
    m_status = status;
    emit statusChanged(status);

    qCDebug(wsClient) << QString("[%1] Status changed from %2 to %3.").arg(
        m_url.toString(),
        QVariant::fromValue(old).toString(),
        QVariant::fromValue(status).toString()
    );

    // Open后启用心跳检测, Closed时停止重连
    // Open后重置重连计数 & 主动关闭标志
    if (status == Open) {
        startHeartbeat();
        clearReconnectAttempts();
        m_manualCloseFlag = false;
        flushPendingMessages();
    }
    else {
        stopHeartbeat();
    }
    if (status == Closed) {
        stopReconnect();
    }

    // 协商子协议
    const auto protocol = m_status == Open && m_socket ? m_socket->subprotocol() : QString();
    if (m_negotiatedSubprotocol != protocol) {
        m_negotiatedSubprotocol = protocol;
        emit negotiatedSubprotocolChanged(protocol);
    }
}

bool WebsocketClient::canSendMessages() const {
    return m_status == Open && m_socket;
}

void WebsocketClient::enqueueTextMessage(const QString& text) {
    m_pendingMessages.enqueue({
        .type = PendingMessage::Text,
        .textPayload = text
    });
}

void WebsocketClient::enqueueBinaryMessage(const QByteArray& data) {
    m_pendingMessages.enqueue({
        .type = PendingMessage::Binary,
        .binaryPayload = data
    });
}

void WebsocketClient::flushPendingMessages() {
    if (!canSendMessages() || m_pendingMessages.isEmpty())
        return;

    while (!m_pendingMessages.isEmpty()) {
        switch (const auto& message = m_pendingMessages.dequeue(); message.type) {
            case PendingMessage::Text:
                m_socket->sendTextMessage(message.textPayload);
                break;
            case PendingMessage::Binary:
                m_socket->sendBinaryMessage(message.binaryPayload);
                break;
        }
    }
}

void WebsocketClient::openSocket() {
    if (m_url.isValid() && m_socket) {
        QWebSocketHandshakeOptions opt;
        opt.setSubprotocols(m_requestedSubprotocols);
        QNetworkRequest request(m_url);
        for (const auto& [name, value] : std::as_const(m_upgradeHeaders))
            request.setRawHeader(name, value);
        m_socket->open(request, opt);
        return;
    }
    qCWarning(wsClient) << QString("Cannot open websocket: invalid URL(%1) or socket.").arg(m_url.toString());
}

void WebsocketClient::startHeartbeat() {
    if (m_heartbeatTimer && m_heartbeatInterval > 0) {
        m_lastPongTimestamp = QDateTime::currentMSecsSinceEpoch();
        m_heartbeatTimer->start(m_heartbeatInterval);
    }
}

void WebsocketClient::stopHeartbeat() {
    if (!m_heartbeatTimer)
        return;
    if (m_heartbeatTimer->isActive()) {
        m_heartbeatTimer->stop();
    }
    m_lastPingTimestamp = 0;
    m_lastPongTimestamp = 0;
}

void WebsocketClient::scheduleReconnect() {
    if (!m_reconnectTimer) {
        setStatus(Closed);
        return;
    }

    // 超过最大重连次数
    if (m_reconnectMaxAttempts > 0 && m_reconnectAttempts >= m_reconnectMaxAttempts) {
        emit errorOccurred(QAbstractSocket::SocketError::RemoteHostClosedError,
            tr("Maximum reconnect attempts reached."));
        setStatus(Closed);
        return;
    }

    if (m_reconnectTimer->isActive())
        return;

    const int delay = nextReconnectDelay();
    addReconnectAttempts();
    setStatus(Reconnecting);
    qCDebug(wsClient) << QString("[%1] Reconnect scheduled in %2 ms (attempt %3)")
                         .arg(url().toString())
                         .arg(delay)
                         .arg(m_reconnectAttempts);

    m_reconnectTimer->start(delay);
}

void WebsocketClient::stopReconnect() const {
    if (m_reconnectTimer && m_reconnectTimer->isActive()) {
        m_reconnectTimer->stop();
    }
}

int WebsocketClient::nextReconnectDelay() const {
    if (m_reconnectBaseDelay <= 0)
        return 0;

    const qint64 factor = static_cast<qint64>(m_reconnectBaseNumber) << m_reconnectAttempts;
    qint64 exponentialDelay = m_reconnectBaseDelay * factor;
    if (exponentialDelay > m_reconnectMaxDelay)
        exponentialDelay = m_reconnectMaxDelay;

    int delay = static_cast<int>(exponentialDelay);
    delay = qMin(m_reconnectMaxDelay, delay);
    delay = qMax(delay, m_reconnectBaseDelay);

    if (!m_useJitter)
        return delay;

    const int jitter = QRandomGenerator::global()->bounded(delay + 1);
    return qMax(jitter, m_reconnectBaseDelay);
}

void WebsocketClient::clearReconnectAttempts() {
    if (m_reconnectAttempts != 0) {
        m_reconnectAttempts = 0;
        emit reconnectAttemptsChanged(0);
    }
}

void WebsocketClient::addReconnectAttempts() {
    ++m_reconnectAttempts;
    emit reconnectAttemptsChanged(m_reconnectAttempts);
}
