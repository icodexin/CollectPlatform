#ifndef WRISTBANDHANDLER_H
#define WRISTBANDHANDLER_H

#include <QAbstractSocket>
#include <QBuffer>
#include <QObject>


class QTcpSocket;
class WristbandData;

class WristbandHandler final : public QObject {
    Q_OBJECT

public:
    explicit WristbandHandler(qintptr socketDescriptor, QObject* parent = nullptr);

signals:
    void clientConnected(const QString& id);                                                               // 连接建立
    void dataReceived(const QString& id, const QByteArray& data);                                       // 收到数据
    void errorOccurred(const QString& id, QAbstractSocket::SocketError error, const QString& errorString); // 发生错误
    void clientDisconnected(const QString& id);                                                            // 连接断开
    void finished();                                                                                       // 任务完成

public slots:
    void start();
    void stop();

private slots:
    void onReadyRead();

private:
    qintptr m_socketDescriptor = 0;
    QTcpSocket* m_socket = nullptr;
    QString m_id;

    QByteArray m_serialized; // 存储序列化后的数据
    QBuffer m_buffer;        // 用于序列化数据的缓冲区
};


#endif //WRISTBANDHANDLER_H
