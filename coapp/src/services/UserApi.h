#ifndef USERAPI_H
#define USERAPI_H

#include <QtCore/QJsonObject>
#include <QtCore/QObject>

#include "singleton.h"

class UserApi final : public QObject, public Singleton<UserApi> {
    Q_OBJECT
    DECLARE_SINGLETON(UserApi)

public:
    static void fetchCurrentUser();

signals:
    void currentUserFetched(const QJsonObject& userInfo);
    void currentUserFetchFailed(int statusCode, const QString& message);
    void currentUserSessionInvalid();

private:
    UserApi() = default;
    ~UserApi() override = default;
};

#endif //USERAPI_H
