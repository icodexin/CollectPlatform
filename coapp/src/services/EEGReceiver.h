#ifndef EEGRECEIVER_H
#define EEGRECEIVER_H

#include <QtNetwork/QTcpSocket>
#include "log.h"
#include "model/EEGData.h"

class EEGDataParser final : public QObject {
    Q_OBJECT

public:
    explicit EEGDataParser(QObject* parent = nullptr);

    void operator()(const QByteArray& raw);
    void parse(const QByteArray& raw);

    void onParsed(const std::function<void(std::unique_ptr<EEGPacket>)>& callback);
    void onParsed(std::function<void(std::unique_ptr<EEGPacket>)>&& callback);

signals:
    // just use this to notify ui or logging system,
    // you need use callback to receive packet data
    void dataParsed(size_t rawSize, size_t packetCount);
    void eventParsed(const EEGEventData& event);

private:
    QByteArray m_token = "@ABCD"; // 数据包起始标记
    QByteArray m_cache;           // 存储未解析的数据
    qint64 m_startLocalTime = 0;  // 第一个数据包的本地毫秒级时间戳
    qint64 m_startDeviceTime = 0; // 第一个数据包的设备毫秒级时间戳
    std::function<void(std::unique_ptr<EEGPacket>)> m_callback; // 解析出EEGPack时的回调

    enum Status {
        Initial,  // 初始状态
        Prepared, // 已准备好接收数据(event_code = 2)
        Receiving // 正在接收数据
    } m_status = Initial;
};

class EEGReceiver final : public QObject {
    Q_OBJECT

public:
    explicit EEGReceiver(QObject* parent = nullptr);

    // register callback function to receive packet in background thread
    void onDataFetched(const std::function<void(std::unique_ptr<EEGPacket>)>& callback);
    void onDataFetched(std::function<void(std::unique_ptr<EEGPacket>)>&& callback);

signals:
    void connected();
    void disconnected();
    void errorOccurred(QAbstractSocket::SocketError error, const QString& errorString);
    void dataFetched(size_t rawSize, size_t packetCount);
    void eventFetched(const EEGEventData& event);
    void logFetched(LogMessage::Level level, const QString& msg);

public slots:
    Q_INVOKABLE void start(const QString& host, quint16 port);
    Q_INVOKABLE void stop();

private slots:
    void onErrorOccurred(QAbstractSocket::SocketError error);

private:
    QTcpSocket* m_socket = nullptr;
    EEGDataParser* m_parser = nullptr;
    QHostAddress m_host;
    quint16 m_port = 8844;
};


#endif //EEGRECEIVER_H
