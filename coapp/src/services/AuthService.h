#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include <QtCore/QByteArray>
#include <QtCore/QJsonObject>
#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QPair>

#include <functional>

#include "singleton.h"

class QNetworkReply;

/// 用户认证服务
class AuthService final : public QObject, public Singleton<AuthService> {
    Q_OBJECT
    DECLARE_SINGLETON(AuthService)

public:
    using RequestFactory = std::function<QNetworkReply*()>;
    using SuccessHandler = std::function<void(QNetworkReply*)>;
    using FailureHandler = std::function<void(int, const QString&, const QByteArray&)>; // code, err_msg, body
    using RefreshSuccessHandler = std::function<void()>;

    struct TokenSet {
        QString accessToken;
        QString refreshToken;
        QString tokenType = QStringLiteral("bearer");

        [[nodiscard]] bool isValid() const {
            return !accessToken.trimmed().isEmpty() && !refreshToken.trimmed().isEmpty();
        }
    };

public:
    /// 是否已认证
    static bool isAuthenticated();
    /// 获取访问Token
    static QString accessToken();
    /// 获取刷新Token
    static QString refreshToken();
    /// 获取Token类型
    static QString tokenType();
    /// 获取当前会话对应的统一身份ID
    static QString unifiedId();

    /// 登录
    static void login(const QString& username, const QString& password);
    /// 注销登录
    static void logout();
    /// 退出会话但不请求注销登录 (保存token)
    static void clearSession();
    /// 对请求添加认证
    static void authorizedRequest(RequestFactory requestFactory, SuccessHandler onSuccess,
        FailureHandler onFailure = {});
    /// 刷新访问Token
    static void refreshTokens(RefreshSuccessHandler onSuccess = {}, FailureHandler onFailure = {});

signals:
    void loginSucceeded();
    void loginFailed(const QString& message);
    void logoutSucceeded();
    void logoutFailed(const QString& message);
    void tokensRefreshed();
    void authenticationChanged(bool authenticated);
    void sessionExpired();

private:
    struct PendingRequest {
        RequestFactory requestFactory;
        SuccessHandler onSuccess;
        FailureHandler onFailure;
    };

    struct PendingRefreshObserver {
        RefreshSuccessHandler onSuccess;
        FailureHandler onFailure;
    };

private:
    AuthService();
    ~AuthService() override = default;

    static QByteArray buildFormBody(const QList<QPair<QString, QString>>& fields);

    void applyStoredAuthorization();
    void saveTokens(const TokenSet& tokenSet);
    void clearTokens();
    void applyAuthorizationHeader() const;
    void queuePendingRequest(PendingRequest pendingRequest);
    void queuePendingRefreshObserver(PendingRefreshObserver observer);
    void flushPendingRequests(bool refreshSucceeded, int statusCode = 0,
        const QString& errorMessage = {}, const QByteArray& responseBody = {});
    void flushPendingRefreshObservers(bool refreshSucceeded, int statusCode = 0,
        const QString& errorMessage = {}, const QByteArray& responseBody = {});
    void handleAuthorizedReply(QNetworkReply* reply, PendingRequest pendingRequest, bool allowRefresh);
    void startRefreshFlow();
    static bool parseTokenReply(QNetworkReply* reply, TokenSet* tokenSet, QString* errorMessage) ;

private:
    TokenSet m_tokens;
    QList<PendingRequest> m_pendingRequests;
    QList<PendingRefreshObserver> m_pendingRefreshObservers;
    bool m_refreshing = false;
};

#endif //AUTHSERVICE_H
