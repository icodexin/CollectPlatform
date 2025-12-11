#include "WebsocketMgr.h"

#include <QtCore/QThread>

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

    auto* client = new WebsocketClient(nullptr);
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

    setClient(client);
    attachSignals(key, client);
    m_clients.insert(key, client);
    return true;
}

bool WebsocketMgr::removeConnection(const QString& key) {
    if (!m_clients.contains(key))
        return false;

    if (auto* c = m_clients.take(key)) {
        removeClient(c);
    }
    return true;
}

void WebsocketMgr::removeAllConnections() {
    for (const auto& [_, client] : std::as_const(m_clients).asKeyValueRange()) {
        removeClient(client);
    }
    m_clients.clear();
}

bool WebsocketMgr::hasConnection(const QString& key) const {
    return m_clients.contains(key);
}

void WebsocketMgr::open(const QString& key) const {
    if (const auto c = client(key))
        openClient(c);
}

void WebsocketMgr::close(const QString& key) const {
    if (const auto c = client(key))
        closeClient(c);
}

void WebsocketMgr::openAll() const {
    for (const auto& [_, c] : std::as_const(m_clients).asKeyValueRange()) {
        openClient(c);
    }
}

void WebsocketMgr::closeAll() {
    for (const auto& [_, c] : std::as_const(m_clients).asKeyValueRange()) {
        closeClient(c);
    }
}

void WebsocketMgr::sendText(const QString& key, const QString& text) const {
    if (const auto c = client(key)) {
        QMetaObject::invokeMethod(c, &WebsocketClient::sendText, text);
    }
}

void WebsocketMgr::sendJson(const QString& key, const QJsonObject& json) const {
    if (const auto c = client(key)) {
        QMetaObject::invokeMethod(c, &WebsocketClient::sendJson, json);
    }
}

void WebsocketMgr::sendBinary(const QString& key, const QByteArray& data) const {
    if (const auto c = client(key)) {
        QMetaObject::invokeMethod(c, &WebsocketClient::sendBinary, data);
    }
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
}

WebsocketMgr::~WebsocketMgr() {
    removeAllConnections();
    if (m_workerThread) {
        m_workerThread->quit();
        m_workerThread->wait();
    }
}

void WebsocketMgr::setClient(WebsocketClient* client) {
    if (!m_workerThread) {
        m_workerThread = new QThread(this);
    }
    client->moveToThread(m_workerThread);
    connect(m_workerThread, &QThread::finished, client, &QObject::deleteLater);
    if (!m_workerThread->isRunning()) {
        m_workerThread->start();
    }
    QMetaObject::invokeMethod(client, &WebsocketClient::init, Qt::QueuedConnection);
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

void WebsocketMgr::openClient(WebsocketClient* client) {
    if (client) {
        QMetaObject::invokeMethod(client, &WebsocketClient::open, Qt::QueuedConnection);
    }
}

void WebsocketMgr::closeClient(WebsocketClient* client) {
    if (client) {
        QMetaObject::invokeMethod(client, &WebsocketClient::close, Qt::QueuedConnection);
    }
}

void WebsocketMgr::removeClient(WebsocketClient* client) {
    if (client) {
        QMetaObject::invokeMethod(client, [client]() {
            client->close();
            client->deleteLater();
        }, Qt::QueuedConnection);
    }
}
