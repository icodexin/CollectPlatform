#ifndef WRISTBANDSERVER_H
#define WRISTBANDSERVER_H

#include <QTcpServer>
#include <QThreadPool>

#include "components/LogEdit.h"


class WristbandClientHandler;
class WristbandData;

class WristbandServer final : public QTcpServer {
    Q_OBJECT

public:
    explicit WristbandServer(QObject* parent = nullptr);
    ~WristbandServer() override;

public slots:
    bool start(quint16 port);
    void stop();

signals:
    void errorOccurred(const QVariantMap& error);                                 // 发生错误
    void clientConnected(const QString& id);                                      // 新客户端连接
    void dataReceived(const QString& id, const QByteArray& data);                 // 收到数据
    void clientDisconnected(const QString& id);                                   // 客户端断开连接
    void runningChanged(bool isRunning);                                          // 服务运行状态变化
    void requestStop();                                                           // 请求停止服务
    void logFetched(const QString& msg, LogEdit::LogLevel level = LogEdit::Info); // 日志消息

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    bool m_running = false;
};


#endif //WRISTBANDSERVER_H
