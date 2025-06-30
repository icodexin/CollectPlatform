#ifndef EEGDATARECEIVER_H
#define EEGDATARECEIVER_H

#include <QTcpSocket>
#include <QBuffer>
#include "model/EEGData.h"
#include "components/LogEdit.h"


class EEGDataReceiver : public QObject {
    Q_OBJECT
public:
    explicit EEGDataReceiver(QObject* parent = nullptr);

signals:
    void connected();

    void eventFetched(EEGEventData event);

    void dataFetched(QByteArray serialized, size_t rawSize, size_t packetNum);

    void disconnected();

    void errorOccurred(QAbstractSocket::SocketError error, QString errorString);

    void logFetched(const QString& msg, LogEdit::LogLevel level = LogEdit::Info);

public slots:
    Q_INVOKABLE void start(const QString& host, quint16 port);

    Q_INVOKABLE void stop() const;

private slots:
    void parse(const QByteArray& raw);

    void onErrorOccurred(QAbstractSocket::SocketError error);

private:
    QHostAddress m_host;
    quint16 m_port = 8844;
    int m_recvSize = 0.04 * 4 * 25 * 300 * 5; // 0.04s * 4 bytes * 25 channels * 300Hz * 5
    QByteArray m_token = "@ABCD"; // 数据包标记
    QTcpSocket* m_socket;
    QByteArray m_cache;      // 存储未解析的数据
    QByteArray m_serialized; // 存储序列化后的数据
    QBuffer m_buffer;        // 用于序列化数据的缓冲区

    enum Status {
        Initial,  // 初始状态
        Prepared, // 已准备好接收数据(event_code = 2)
        Receiving // 正在接收数据
    } m_status = Initial;

    qint64 m_startLocalTime = 0;  // 第一个数据包的本地毫秒级时间戳
    qint64 m_startDeviceTime = 0; // 第一个数据包的设备毫秒级时间戳
};


#endif //EEGDATARECEIVER_H
