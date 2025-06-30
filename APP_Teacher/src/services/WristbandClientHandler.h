#ifndef WRISTBANDCLIENTHANDLER_H
#define WRISTBANDCLIENTHANDLER_H

#include <QRunnable>
#include <QAbstractSocket>
#include "WristbandData.h"
#include <QTcpSocket>


class WristbandClientHandler : public QObject, public QRunnable {
    Q_OBJECT

public:
    explicit WristbandClientHandler(qintptr socketDescriptor, QObject* parent = nullptr);

signals:
    void clientConnected(const QString& id);                         // 连接建立
    void dataReceived(const QString& id, const WristbandData& data); // 收到数据
    void errorOccurred(const QString& id, const QString& error);     // 发生错误
    void clientDisconnected(const QString& id);                      // 连接断开
    void requestDisconnect();                                        // 请求断开连接

public slots:
    void disconnectClient();

protected:
    void run() override;

private:
    qintptr m_socketDescriptor;
    QTcpSocket* m_socket = nullptr;
    QString m_id;

    void parseData(const QByteArray& data);
};


#endif //WRISTBANDCLIENTHANDLER_H
