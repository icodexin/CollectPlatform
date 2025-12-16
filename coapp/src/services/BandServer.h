#ifndef BANDSERVER_H
#define BANDSERVER_H

#include <QtNetwork/QTcpServer>

class WristbandPacket;

class BandClientHandler final : public QObject {
    Q_OBJECT

public:
    explicit BandClientHandler(qintptr socketDescriptor, QObject* parent = nullptr);

signals:
    void clientConnected(const QString& id);                                                               // 连接建立
    void clientDisconnected(const QString& id);                                                            // 连接断开
    void dataReceived(const QString& id, const WristbandPacket& data);                                     // 收到数据
    void errorOccurred(const QString& id, QAbstractSocket::SocketError error, const QString& errorString); // 发生错误
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
};

class BandServer final : public QTcpServer {
    Q_OBJECT

public:
    explicit BandServer(QObject* parent = nullptr);
    ~BandServer() override;

    void onDataReceived(const std::function<void(std::unique_ptr<WristbandPacket>)>& callback);
    void onDataReceived(std::function<void(std::unique_ptr<WristbandPacket>)>&& callback);

signals:
    void started();                                         // 服务已启动
    void stopped();                                         // 服务已停止
    void requestStop();                                     // 请求停止服务
    void runningChanged(bool isRunning);                    // 服务运行状态变化
    void errorOccurred(const QVariantMap& error);           // 发生错误
    void clientConnected(const QString& id);                // 新客户端连接
    void clientDisconnected(const QString& id);             // 客户端断开连接
    void dataReceived(const QString& id, qsizetype length); // 收到数据

public slots:
    bool start(quint16 port);
    void stop();

private slots:
    void setRunning(bool);
    void onClientErrorOccurred(const QString& id, QAbstractSocket::SocketError error, const QString& errorString);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private:
    bool m_running = false;
    std::function<void(std::unique_ptr<WristbandPacket>)> m_callback;
};


#endif //BANDSERVER_H
