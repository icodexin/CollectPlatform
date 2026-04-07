#ifndef HTTPMGR_H
#define HTTPMGR_H

#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>

#include "singleton.h"

class QJsonDocument;
class QNetworkReply;
class QNetworkRequest;
class QSslError;

class HttpMgr final : public QObject, public Singleton<HttpMgr> {
    Q_OBJECT
    DECLARE_SINGLETON(HttpMgr)

public:
    static void setBaseUrl(const QUrl& baseUrl);
    static void setBaseUrl(const QString& baseUrl);
    static QUrl baseUrl();

    static void setTransferTimeout(int timeoutMs);
    static int transferTimeout();

    static void setDefaultHeader(const QByteArray& name, const QByteArray& value);
    static void removeDefaultHeader(const QByteArray& name);
    static void clearDefaultHeaders();
    static void setBearerToken(const QString& token);

    static QNetworkReply* get(const QString& path, const QUrlQuery& query = {});
    static QNetworkReply* post(const QString& path, const QByteArray& body = {},
        const QByteArray& contentType = QByteArrayLiteral("application/json"));
    static QNetworkReply* postJson(const QString& path, const QJsonDocument& json,
        const QUrlQuery& query = {});
    static QNetworkReply* put(const QString& path, const QByteArray& body = {},
        const QByteArray& contentType = QByteArrayLiteral("application/json"));
    static QNetworkReply* putJson(const QString& path, const QJsonDocument& json,
        const QUrlQuery& query = {});
    static QNetworkReply* deleteResource(const QString& path, const QUrlQuery& query = {});
    static QNetworkReply* request(const QByteArray& method, const QString& path,
        const QByteArray& body = {}, const QUrlQuery& query = {},
        const QByteArray& contentType = {});

signals:
    void requestStarted(const QString& method, const QUrl& url);
    void requestFinished(const QString& method, const QUrl& url, int statusCode);
    void requestFailed(const QString& method, const QUrl& url, int statusCode, const QString& errorString);
    void sslErrorsOccurred(const QUrl& url, const QStringList& errors);

private slots:
    void onReplyFinished(QNetworkReply* reply);
    void onSslErrors(QNetworkReply* reply, const QList<QSslError>& errors);

private:
    HttpMgr();
    ~HttpMgr() override = default;

    QUrl buildUrl(const QString& path, const QUrlQuery& query) const;
    QNetworkRequest buildRequest(const QString& path, const QUrlQuery& query,
        const QByteArray& contentType) const;
    void applyDefaultHeaders(QNetworkRequest& request) const;
    QNetworkReply* send(const QByteArray& method, const QString& path, const QByteArray& body,
        const QUrlQuery& query, const QByteArray& contentType);

private:
    QNetworkAccessManager m_accessManager;
    QUrl m_baseUrl;
    QHash<QByteArray, QByteArray> m_defaultHeaders;
    int m_transferTimeoutMs = 15000;
};

#endif //HTTPMGR_H
