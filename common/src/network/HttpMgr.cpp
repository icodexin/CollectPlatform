#include "network/HttpMgr.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QStringList>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QSslError>

namespace {
constexpr auto kMethodAttribute = QNetworkRequest::User;
}

HttpMgr::HttpMgr() {
    m_defaultHeaders.insert("Accept", "application/json");

    connect(&m_accessManager, &QNetworkAccessManager::finished, this, &HttpMgr::onReplyFinished);
    connect(&m_accessManager, &QNetworkAccessManager::sslErrors, this, &HttpMgr::onSslErrors);
}

void HttpMgr::setBaseUrl(const QUrl& baseUrl) {
    QUrl normalizedUrl(baseUrl);
    if (normalizedUrl.isValid()) {
        QString path = normalizedUrl.path();
        if (path.isEmpty())
            path = "/";
        if (!path.endsWith('/'))
            path += '/';
        normalizedUrl.setPath(path);
    }

    instance().m_baseUrl = normalizedUrl;
}

void HttpMgr::setBaseUrl(const QString& baseUrl) {
    setBaseUrl(QUrl(baseUrl));
}

QUrl HttpMgr::baseUrl() {
    return instance().m_baseUrl;
}

void HttpMgr::setTransferTimeout(const int timeoutMs) {
    instance().m_transferTimeoutMs = qMax(0, timeoutMs);
}

int HttpMgr::transferTimeout() {
    return instance().m_transferTimeoutMs;
}

void HttpMgr::setDefaultHeader(const QByteArray& name, const QByteArray& value) {
    if (name.isEmpty())
        return;

    instance().m_defaultHeaders.insert(name, value);
}

void HttpMgr::removeDefaultHeader(const QByteArray& name) {
    instance().m_defaultHeaders.remove(name);
}

void HttpMgr::clearDefaultHeaders() {
    instance().m_defaultHeaders.clear();
}

void HttpMgr::setBearerToken(const QString& token) {
    if (token.trimmed().isEmpty()) {
        removeDefaultHeader("Authorization");
        return;
    }

    setDefaultHeader("Authorization", "Bearer " + token.trimmed().toUtf8());
}

QNetworkReply* HttpMgr::get(const QString& path, const QUrlQuery& query) {
    return instance().send("GET", path, {}, query, {});
}

QNetworkReply* HttpMgr::post(const QString& path, const QByteArray& body, const QByteArray& contentType) {
    return instance().send("POST", path, body, {}, contentType);
}

QNetworkReply* HttpMgr::postJson(const QString& path, const QJsonDocument& json, const QUrlQuery& query) {
    return instance().send("POST", path, json.toJson(QJsonDocument::Compact), query, "application/json");
}

QNetworkReply* HttpMgr::put(const QString& path, const QByteArray& body, const QByteArray& contentType) {
    return instance().send("PUT", path, body, {}, contentType);
}

QNetworkReply* HttpMgr::putJson(const QString& path, const QJsonDocument& json, const QUrlQuery& query) {
    return instance().send("PUT", path, json.toJson(QJsonDocument::Compact), query, "application/json");
}

QNetworkReply* HttpMgr::deleteResource(const QString& path, const QUrlQuery& query) {
    return instance().send("DELETE", path, {}, query, {});
}

QNetworkReply* HttpMgr::request(const QByteArray& method, const QString& path,
    const QByteArray& body, const QUrlQuery& query, const QByteArray& contentType) {
    return instance().send(method.trimmed().toUpper(), path, body, query, contentType);
}

void HttpMgr::onReplyFinished(QNetworkReply* reply) {
    if (!reply)
        return;

    const QString method = reply->request().attribute(kMethodAttribute).toString();
    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    const QUrl url = reply->url();

    if (reply->error() == QNetworkReply::NoError) {
        emit requestFinished(method, url, statusCode);
        return;
    }

    emit requestFailed(method, url, statusCode, reply->errorString());
}

void HttpMgr::onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors) {
    if (!reply)
        return;

    QStringList messages;
    messages.reserve(errors.size());
    for (const auto& error : errors)
        messages.push_back(error.errorString());

    emit sslErrorsOccurred(reply->url(), messages);
}

QUrl HttpMgr::buildUrl(const QString& path, const QUrlQuery& query) const {
    QUrl url(path);
    if (url.isRelative() || url.scheme().isEmpty()) {
        if (m_baseUrl.isValid() && !m_baseUrl.isEmpty())
            url = m_baseUrl.resolved(QUrl(path));
    }

    if (!query.isEmpty()) {
        QUrlQuery mergedQuery(url);
        const auto items = query.queryItems(QUrl::FullyDecoded);
        for (const auto& item : items)
            mergedQuery.addQueryItem(item.first, item.second);
        url.setQuery(mergedQuery);
    }

    return url;
}

QNetworkRequest HttpMgr::buildRequest(const QString& path, const QUrlQuery& query,
    const QByteArray& contentType) const {
    QNetworkRequest request(buildUrl(path, query));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    if (m_transferTimeoutMs > 0)
        request.setTransferTimeout(m_transferTimeoutMs);

    applyDefaultHeaders(request);

    if (!contentType.isEmpty())
        request.setHeader(QNetworkRequest::ContentTypeHeader, QString::fromUtf8(contentType));

    return request;
}

void HttpMgr::applyDefaultHeaders(QNetworkRequest& request) const {
    for (auto it = m_defaultHeaders.cbegin(); it != m_defaultHeaders.cend(); ++it)
        request.setRawHeader(it.key(), it.value());
}

QNetworkReply* HttpMgr::send(const QByteArray& method, const QString& path, const QByteArray& body,
    const QUrlQuery& query, const QByteArray& contentType) {
    const QByteArray normalizedMethod = method.trimmed().toUpper();
    QNetworkRequest request = buildRequest(path, query, contentType);
    request.setAttribute(kMethodAttribute, QString::fromLatin1(normalizedMethod));

    emit requestStarted(QString::fromLatin1(normalizedMethod), request.url());

    if (normalizedMethod == "GET")
        return m_accessManager.get(request);
    if (normalizedMethod == "POST")
        return m_accessManager.post(request, body);
    if (normalizedMethod == "PUT")
        return m_accessManager.put(request, body);
    if (normalizedMethod == "DELETE" && body.isEmpty())
        return m_accessManager.deleteResource(request);

    return m_accessManager.sendCustomRequest(request, normalizedMethod, body);
}
