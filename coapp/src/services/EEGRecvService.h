#ifndef EEGRECVSERVICE_H
#define EEGRECVSERVICE_H

#include <functional>

#include <QtCore/QObject>
#include <QtNetwork/QAbstractSocket>

#include "log.h"
#include "concurrency/EventLoopWorkerHost.h"
#include "model/EEGData.h"

class IEventLoopThreadProvider;
class QTcpSocket;

using EEGPacketHandler = std::function<void(std::unique_ptr<EEGPacket>)>;

struct EEGParseResult {
    qsizetype rawSize = 0;
    QList<EEGSensorData> data;
    QList<EEGEventData> events;

    bool hasData() const {
        return !data.isEmpty();
    }

    bool hasEvents() const {
        return !events.isEmpty();
    }
};

class EEGDataParser {
public:
    EEGParseResult parse(const QByteArray& raw);
    void reset();

private:
    QByteArray m_cache;          // 存储未解析的数据
    qint64 m_startLocalTime = 0; // 第一个数据包的本地毫秒级时间戳
    qint64 m_startDeviceTime = 0;// 第一个数据包的设备毫秒级时间戳
    bool m_hasTimeBase = false;  // 是否已建立本地/设备时间基准

    enum Status {
        Initial,  // 初始状态
        Prepared, // 已准备好接收数据(event_code = 2)
        Receiving // 正在接收数据
    } m_status = Initial;
};

class EEGRecvWorker final : public QObject {
    Q_OBJECT

public:
    explicit EEGRecvWorker(QObject* parent = nullptr);

    void onDataFetched(EEGPacketHandler callback);

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
    void onConnected();
    void onDisconnected();
    void onErrorOccurred(QAbstractSocket::SocketError error);
    void onReadyRead();

private:
    void cleanupSocket();

private:
    QTcpSocket* m_socket = nullptr;
    EEGDataParser m_parser;
    EEGPacketHandler m_dataHandler;
    QString m_host;
    quint16 m_port = 8844;
};

class EEGRecvService final : public QObject, private EventLoopWorkerHost<EEGRecvWorker> {
    Q_OBJECT

public:
    explicit EEGRecvService(QObject* parent = nullptr);
    explicit EEGRecvService(IEventLoopThreadProvider* threadProvider, QObject* parent = nullptr);
    ~EEGRecvService() override;

    // 在后台线程中注册回调函数, 以接收数据包
    void onDataFetched(EEGPacketHandler callback);

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

private:
    EEGRecvService(IEventLoopThreadProvider* threadProvider, bool ownsThreadProvider, QObject* parent);
    void ensureWorkerCreated();
    void bindWorkerSignals(EEGRecvWorker* worker);

private:
    bool m_ownsThreadProvider = false;
    EEGPacketHandler m_dataFetchedCallback;
};

#endif // EEGRECVSERVICE_H
