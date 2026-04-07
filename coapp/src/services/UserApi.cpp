#include "UserApi.h"

#include <QtCore/QJsonDocument>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include "AuthService.h"
#include "network/HttpMgr.h"

void UserApi::fetchCurrentUser() {
    auto& api = instance();

    AuthService::authorizedRequest(
        [] {
            return HttpMgr::get("/api/users/me");
        },
        [&api](QNetworkReply* reply) {
            const auto doc = QJsonDocument::fromJson(reply->readAll());
            if (!doc.isObject()) {
                emit api.currentUserFetchFailed(
                    reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(),
                    QStringLiteral("Unexpected user payload."));
                return;
            }

            emit api.currentUserFetched(doc.object());
        },
        [&api](const int statusCode, const QString& message, const QByteArray&) {
            if (statusCode == 403) {
                AuthService::clearSession();
                emit api.currentUserSessionInvalid();
            }
            emit api.currentUserFetchFailed(statusCode, message);
        });
}
