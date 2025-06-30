#ifndef WRISTBANDLISTENER_H
#define WRISTBANDLISTENER_H

#include <QDebug>
#include <QTcpServer>
#include <QMutex>
#include "WristbandData.h"
#include "WristbandClientHandler.h"

class WristbandListener : public QTcpServer {
    Q_OBJECT
    Q_PROPERTY(bool listening READ isListening NOTIFY listeningChanged)
    Q_PROPERTY(quint16 port READ serverPort)
    QML_ELEMENT

public:
    explicit WristbandListener(QObject* parent = nullptr);

    ~WristbandListener() override;

    Q_INVOKABLE bool startServer(quint16 port);

    Q_INVOKABLE void stopServer();

signals:
    void errorOccurred(const QVariantMap& error);                    // 发生错误
    void clientConnected(const QString& id);                         // 新客户端连接
    void dataReceived(const QString& id, const WristbandData& data); // 收到数据
    void clientDisconnected(const QString& id);                      // 客户端断开连接
    void listeningChanged(bool isListening);                         // 监听状态变化
    void requestDisconnect(qintptr socketDescriptor);                // 请求断开连接

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    QMutex m_mutex;
    QMap<qintptr, QPointer<WristbandClientHandler>> m_activeHandlers;
};

#endif //WRISTBANDLISTENER_H
