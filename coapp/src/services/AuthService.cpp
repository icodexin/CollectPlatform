#include "AuthService.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include "CoSettingsMgr.h"
#include "network/HttpMgr.h"

namespace {
    QString extractErrorMessage(const QNetworkReply::NetworkError errorCode, const int statusCode, const QByteArray& body,
                                const QString& fallbackMessage) {
        // Body包含错误信息, 从Body中读取错误信息
        if (const auto doc = QJsonDocument::fromJson(body); doc.isObject()) {
            const auto obj = doc.object();
            const QStringList candidateKeys = {"detail", "message", "error", "msg"};
            for (const auto& key : candidateKeys) {
                const auto value = obj.value(key);
                if (value.isString() && !value.toString().trimmed().isEmpty())
                    return value.toString().trimmed();
            }
        }

        // 从状态码中解析错误信息
        if (statusCode != 0) {
            switch (statusCode) {
                case 401:
                    return QCoreApplication::translate("AuthService", "User is unauthorized.");
                case 403:
                    return QCoreApplication::translate("AuthService", "Operation is forbidden.");
                case 502:
                    return QCoreApplication::translate("AuthService", "Gateway Error.");
                default:
                    // 其他状态码跳出
                    break;
            }
        }

        // 从ErrorCode解析错误信息
        switch (errorCode) {
            case QNetworkReply::NoError:
                // 无错误跳出
                break;
            case QNetworkReply::ConnectionRefusedError:
            case QNetworkReply::RemoteHostClosedError:
                return QCoreApplication::translate("AuthService", "Unable to connect to the server. "
                    "Please confirm whether the backend service has been started.");
            case QNetworkReply::HostNotFoundError:
                return QCoreApplication::translate("AuthService", "The server address cannot be parsed. "
                    "Please check your network configuration.");
            case QNetworkReply::TimeoutError:
                return QCoreApplication::translate("AuthService", "Request timed out, "
                    "please check your network connection.");
            default:
                return fallbackMessage;
        }

        // 兜底返回
        if (!body.trimmed().isEmpty())
            return QString::fromUtf8(body);
        return QStringLiteral("Request failed.");
    }
}


AuthService::AuthService() {
    m_tokens.accessToken = CoSettingsMgr::authAccessToken();
    m_tokens.refreshToken = CoSettingsMgr::authRefreshToken();
    m_tokens.tokenType = CoSettingsMgr::authTokenType();
    applyStoredAuthorization();
}

bool AuthService::isAuthenticated() {
    return !instance().m_tokens.accessToken.trimmed().isEmpty();
}

QString AuthService::accessToken() {
    return instance().m_tokens.accessToken;
}

QString AuthService::refreshToken() {
    return instance().m_tokens.refreshToken;
}

QString AuthService::tokenType() {
    return instance().m_tokens.tokenType;
}

QString AuthService::unifiedId() {
    return CoSettingsMgr::authUnifiedId();
}

void AuthService::login(const QString& username, const QString& password) {
    auto& service = instance();

    auto* reply = HttpMgr::post("/auth/token",
        buildFormBody({
            {"username", username.trimmed()},
            {"password", password},
        }),
        "application/x-www-form-urlencoded");

    QObject::connect(reply, &QNetworkReply::finished, &service, [&service, reply] {
        TokenSet tokenSet;
        QString errorMessage;
        if (service.parseTokenReply(reply, &tokenSet, &errorMessage)) {
            const bool wasAuthenticated = !service.m_tokens.accessToken.trimmed().isEmpty();
            service.saveTokens(tokenSet);
            emit service.loginSucceeded();
            if (!wasAuthenticated)
                emit service.authenticationChanged(true);
        }
        else {
            emit service.loginFailed(errorMessage);
        }

        reply->deleteLater();
    });
}

void AuthService::logout() {
    auto& service = instance();

    if (service.m_tokens.refreshToken.trimmed().isEmpty()) {
        const bool wasAuthenticated = isAuthenticated();
        service.clearTokens();
        emit service.logoutSucceeded();
        if (wasAuthenticated)
            emit service.authenticationChanged(false);
        return;
    }

    auto* reply = HttpMgr::post("/auth/logout",
        buildFormBody({
            {"refresh_token", service.m_tokens.refreshToken},
        }),
        "application/x-www-form-urlencoded");

    QObject::connect(reply, &QNetworkReply::finished, &service, [&service, reply] {
        const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const QByteArray body = reply->readAll();
        const auto errCode = reply->error();
        const bool requestOk = errCode && statusCode >= 200 && statusCode < 300;
        const bool wasAuthenticated = !service.m_tokens.accessToken.trimmed().isEmpty();

        service.clearTokens();
        if (requestOk) {
            emit service.logoutSucceeded();
        }
        else {
            emit service.logoutFailed(extractErrorMessage(errCode, statusCode, body, reply->errorString()));
        }

        if (wasAuthenticated)
            emit service.authenticationChanged(false);

        reply->deleteLater();
    });
}

void AuthService::clearSession() {
    auto& service = instance();
    const bool wasAuthenticated = !service.m_tokens.accessToken.trimmed().isEmpty();
    service.clearTokens();
    if (wasAuthenticated)
        emit service.authenticationChanged(false);
}

void AuthService::authorizedRequest(RequestFactory requestFactory, SuccessHandler onSuccess, FailureHandler onFailure) {
    auto& service = instance();
    PendingRequest pendingRequest{std::move(requestFactory), std::move(onSuccess), std::move(onFailure)};
    if (!pendingRequest.requestFactory) {
        if (pendingRequest.onFailure)
            pendingRequest.onFailure(0, QStringLiteral("Invalid request factory."), {});
        return;
    }

    auto* reply = pendingRequest.requestFactory();
    if (!reply) {
        if (pendingRequest.onFailure)
            pendingRequest.onFailure(0, QStringLiteral("Failed to create request."), {});
        return;
    }

    service.handleAuthorizedReply(reply, std::move(pendingRequest), true);
}

QByteArray AuthService::buildFormBody(const QList<QPair<QString, QString>>& fields) {
    QUrlQuery query;
    for (const auto& [key, value] : fields)
        query.addQueryItem(key, value);
    return query.toString(QUrl::FullyEncoded).toUtf8();
}

void AuthService::applyStoredAuthorization() {
    applyAuthorizationHeader();
}

void AuthService::saveTokens(const TokenSet& tokenSet) {
    m_tokens = tokenSet;
    if (m_tokens.tokenType.trimmed().isEmpty())
        m_tokens.tokenType = QStringLiteral("bearer");

    CoSettingsMgr::setAuthAccessToken(m_tokens.accessToken);
    CoSettingsMgr::setAuthRefreshToken(m_tokens.refreshToken);
    CoSettingsMgr::setAuthTokenType(m_tokens.tokenType);
    CoSettingsMgr::flush();

    applyAuthorizationHeader();
}

void AuthService::clearTokens() {
    m_tokens = {};
    m_tokens.tokenType = QStringLiteral("bearer");
    CoSettingsMgr::setAuthAccessToken({});
    CoSettingsMgr::setAuthRefreshToken({});
    CoSettingsMgr::setAuthTokenType(QStringLiteral("bearer"));
    CoSettingsMgr::setAuthUnifiedId({});
    CoSettingsMgr::flush();
    HttpMgr::removeDefaultHeader("Authorization");
}

void AuthService::applyAuthorizationHeader() const {
    const QString normalizedType = m_tokens.tokenType.trimmed().isEmpty()
                                       ? QStringLiteral("bearer")
                                       : m_tokens.tokenType.trimmed();
    const QString authScheme = normalizedType.compare(QStringLiteral("bearer"), Qt::CaseInsensitive) == 0
                                   ? QStringLiteral("Bearer")
                                   : normalizedType;

    if (m_tokens.accessToken.trimmed().isEmpty()) {
        HttpMgr::removeDefaultHeader("Authorization");
        return;
    }

    HttpMgr::setDefaultHeader("Authorization",
        authScheme.toUtf8() + ' ' + m_tokens.accessToken.trimmed().toUtf8());
}

void AuthService::queuePendingRequest(PendingRequest pendingRequest) {
    m_pendingRequests.push_back(std::move(pendingRequest));
}

void AuthService::flushPendingRequests(const bool refreshSucceeded, const int statusCode,
                                       const QString& errorMessage, const QByteArray& responseBody) {
    auto pendingRequests = std::move(m_pendingRequests);
    m_pendingRequests.clear();

    for (auto& pendingRequest : pendingRequests) {
        if (refreshSucceeded) {
            auto* retryReply = pendingRequest.requestFactory ? pendingRequest.requestFactory() : nullptr;
            if (!retryReply) {
                if (pendingRequest.onFailure)
                    pendingRequest.onFailure(0, QStringLiteral("Failed to recreate authorized request."), {});
                continue;
            }

            handleAuthorizedReply(retryReply, std::move(pendingRequest), false);
            continue;
        }

        if (pendingRequest.onFailure)
            pendingRequest.onFailure(statusCode, errorMessage, responseBody);
    }
}

void AuthService::handleAuthorizedReply(QNetworkReply* reply, PendingRequest pendingRequest, const bool allowRefresh) {
    if (!reply) {
        if (pendingRequest.onFailure)
            pendingRequest.onFailure(0, QStringLiteral("Invalid network reply."), {});
        return;
    }

    QObject::connect(reply, &QNetworkReply::finished, this,
        [this, reply, pendingRequest = std::move(pendingRequest), allowRefresh]() mutable {
            const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

            // 未授权访问, 重新登录
            if (allowRefresh && statusCode == 401) {
                const bool canRefresh = !m_tokens.refreshToken.trimmed().isEmpty();
                reply->deleteLater();

                // 不可刷新token时, 此次request失败
                if (!canRefresh) {
                    clearTokens();
                    emit authenticationChanged(false);
                    emit sessionExpired();
                    if (pendingRequest.onFailure) {
                        pendingRequest.onFailure(401, QStringLiteral("Authentication expired."), {});
                    }
                    return;
                }

                // 将当前request排队等待, 在刷新token后重新请求
                queuePendingRequest(std::move(pendingRequest));
                startRefreshFlow();
                return;
            }

            // 没有错误
            const auto errCode = reply->error();
            if (errCode == QNetworkReply::NoError && statusCode >= 200 && statusCode < 300) {
                if (pendingRequest.onSuccess)
                    pendingRequest.onSuccess(reply);
            }
            // 其他情况
            else if (pendingRequest.onFailure) {
                const QByteArray body = reply->readAll();
                const QString errMsg = extractErrorMessage(errCode, statusCode, body, reply->errorString());
                pendingRequest.onFailure(statusCode, errMsg, body);
            }

            reply->deleteLater();
        });
}

void AuthService::startRefreshFlow() {
    // 保证同一时刻只发一次refresh
    if (m_refreshing)
        return;

    // 检查刷新Token
    if (m_tokens.refreshToken.trimmed().isEmpty()) {
        clearTokens();
        emit authenticationChanged(false);
        emit sessionExpired();
        flushPendingRequests(false, 401, QStringLiteral("Authentication expired."), {});
        return;
    }

    m_refreshing = true;

    // 调用refresh接口
    auto* reply = HttpMgr::post("/auth/refresh",
        buildFormBody({
            {"refresh_token", m_tokens.refreshToken},
        }),
        "application/x-www-form-urlencoded");

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply] {
        m_refreshing = false;

        TokenSet tokenSet;
        QString errorMessage;
        if (parseTokenReply(reply, &tokenSet, &errorMessage)) {
            saveTokens(tokenSet);
            emit tokensRefreshed();
            flushPendingRequests(true);
        }
        else {
            clearTokens();
            emit authenticationChanged(false);
            emit sessionExpired();
            flushPendingRequests(false,
                reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                errorMessage,
                {});
        }

        reply->deleteLater();
    });
}

bool AuthService::parseTokenReply(QNetworkReply* reply, TokenSet* tokenSet, QString* errorMessage) {
    if (!reply || !tokenSet || !errorMessage)
        return false;

    const QByteArray body = reply->readAll();
    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError || statusCode < 200 || statusCode >= 300) {
        *errorMessage = extractErrorMessage(reply->error(), statusCode, body, reply->errorString());
        return false;
    }

    const auto doc = QJsonDocument::fromJson(body);
    if (!doc.isObject()) {
        *errorMessage = QStringLiteral("Unexpected token response.");
        return false;
    }

    const auto obj = doc.object();
    tokenSet->accessToken = obj.value("access_token").toString().trimmed();
    tokenSet->refreshToken = obj.value("refresh_token").toString().trimmed();
    tokenSet->tokenType = obj.value("token_type").toString().trimmed();
    if (tokenSet->tokenType.isEmpty())
        tokenSet->tokenType = QStringLiteral("bearer");

    if (!tokenSet->isValid()) {
        *errorMessage = QStringLiteral("Incomplete token response.");
        return false;
    }

    return true;
}
