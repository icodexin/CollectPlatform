#include "WebsocketMgr.h"

#include <QtGui/QGuiApplication>

void WebsocketMgr::setHeartbeatParams(const int interval, const int timeout, const int retries) {
    m_heartbeatParams["interval"] = interval;
    m_heartbeatParams["timeout"] = timeout;
    m_heartbeatParams["retries"] = retries;
}

void WebsocketMgr::setReconnectParams(const int maxAttempts, const int baseDelay, const int maxDelay,
                                     const bool useJitter) {
    m_reconnectParams["maxAttempts"] = maxAttempts;
    m_reconnectParams["baseDelay"] = baseDelay;
    m_reconnectParams["maxDelay"] = maxDelay;
    m_reconnectParams["useJitter"] = useJitter;
}

bool WebsocketMgr::createConnection(const QString& key, const QUrl& url) {
    if (key.isEmpty() || !url.isValid())
        return false;

    if (m_clients.contains(key))
        return false;

    auto* client = new WebsocketClient(this);
    client->setUrl(url);
    client->setHeartbeat(
        m_heartbeatParams.value("interval", WebsocketClient::defaultHeartbeatInterval).toInt(),
        m_heartbeatParams.value("timeout", WebsocketClient::defaultHeartbeatTimeout).toInt(),
        m_heartbeatParams.value("retries", WebsocketClient::defaultHeartbeatRetries).toInt()
    );
    client->setReconnect(
        m_reconnectParams.value("maxAttempts", WebsocketClient::defaultReconnectMaxAttempts).toInt(),
        m_reconnectParams.value("baseDelay", WebsocketClient::defaultReconnectBaseDelay).toInt(),
        m_reconnectParams.value("maxDelay", WebsocketClient::defaultReconnectMaxDelay).toInt(),
        m_reconnectParams.value("useJitter", WebsocketClient::defaultUseJitter).toBool()
    );

    if (!m_authToken.isEmpty()) {
        WebsocketClient::HttpHeaders headers = client->upgradeHeaders();
        headers.push_back({"Authorization", m_authToken.toUtf8()});
        client->setUpgradeHeaders(headers);
    }

    attachSignals(key, client);
    m_clients.insert(key, client);
    return true;
}

bool WebsocketMgr::removeConnection(const QString& key) {
    if (!m_clients.contains(key))
        return false;

    if (auto* c = m_clients.take(key)) {
        c->close();
        c->deleteLater();
    }
    return true;
}

void WebsocketMgr::removeAllConnections() {
    for (const auto& [_, client] : std::as_const(m_clients).asKeyValueRange()) {
        if (client) {
            client->close();
            client->deleteLater();
        }
    }
    m_clients.clear();
}

bool WebsocketMgr::hasConnection(const QString& key) const {
    return m_clients.contains(key);
}

void WebsocketMgr::open(const QString& key) const {
    if (const auto c = client(key))
        c->open();
}

void WebsocketMgr::close(const QString& key) const {
    if (const auto c = client(key))
        c->close();
}

void WebsocketMgr::openAll() const {
    for (const auto& [_, client] : std::as_const(m_clients).asKeyValueRange()) {
        if (client)
            client->open();
    }
}

void WebsocketMgr::closeAll() {
    for (const auto& [_, client] : std::as_const(m_clients).asKeyValueRange()) {
        if (client)
            client->close();
    }
}

qint64 WebsocketMgr::sendText(const QString& key, const QString& text) const {
    if (const auto c = client(key))
        return c->sendText(text);
    return 0;
}

qint64 WebsocketMgr::sendJson(const QString& key, const QJsonObject& json) const {
    if (const auto c = client(key))
        return c->sendJson(json);
    return 0;
}

qint64 WebsocketMgr::sendBinary(const QString& key, const QByteArray& data) const {
    if (const auto c = client(key))
        return c->sendBinary(data);
    return 0;
}

QPointer<WebsocketClient> WebsocketMgr::client(const QString& key) const {
    return m_clients.value(key, nullptr);
}

QString WebsocketMgr::authToken() const {
    return m_authToken;
}

void WebsocketMgr::setAuthToken(const QString& token) {
    if (m_authToken == token)
        return;
    m_authToken = token;
}

WebsocketMgr::WebsocketMgr(QObject* parent) : QObject(parent) {
    connect(qApp, &QCoreApplication::aboutToQuit, this, &WebsocketMgr::removeAllConnections);
}

WebsocketMgr::~WebsocketMgr() {
    closeAll();
    qDeleteAll(m_clients);
    m_clients.clear();
}

void WebsocketMgr::attachSignals(const QString& key, const WebsocketClient* client) {
    connect(client, &WebsocketClient::textReceived, this, [this, key](const QString& text) {
        emit textReceived(key, text);
    });
    connect(client, &WebsocketClient::binaryReceived, this, [this, key](const QByteArray& data) {
        emit binaryReceived(key, data);
    });
    connect(client, &WebsocketClient::statusChanged, this, [this, key](const WebsocketClient::Status stats) {
        emit statsChanged(key, stats);
    });
    connect(client, &WebsocketClient::errorOccurred, this,
        [this, key](const QAbstractSocket::SocketError error, const QString& errorString) {
            emit errorOccurred(key, error, errorString);
        }
    );
}
