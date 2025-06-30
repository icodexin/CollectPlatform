#include "EEGDataReceiver.h"
#include <QtEndian>
#include <QDateTime>
#include "model/serialize.h"
#include <QBuffer>

// 原始数据的通道顺序
static std::array<QString, 24> inChannels = {
    "P3", "C3", "F3", "Fz", "F4", "C4", "P4", "Cz", "CM", "A1",
    "Fp1", "Fp2", "T3", "T5", "O1", "O2", "X3", "X2", "F7", "F8",
    "X1", "A2", "T6", "T4"
};

// 输出数据的通道顺序
static std::array<QString, 24> outChannels = {
    "Fp1", "Fp2", "F7", "F3", "Fz", "F4", "F8", "A1", "T3", "C3",
    "Cz", "C4", "T4", "A2", "T5", "P3", "P4", "T6", "O1", "O2",
    "CM", "X1", "X2", "X3"
};

static std::array<int, 24> getChReorderMap() {
    std::array<int, 24> reorderMap{};
    for (int i = 0; i < outChannels.size(); ++i) {
        auto it = std::find(inChannels.begin(), inChannels.end(), outChannels[i]);
        if (it != inChannels.end()) {
            reorderMap[i] = std::distance(inChannels.begin(), it);
        } else {
            qWarning() << "Channel not found in original order:" << inChannels[i];
        }
    }
    return reorderMap;
}

// 通道重排序映射
static std::array<int, 24> chReorderMap = getChReorderMap();

EEGDataReceiver::EEGDataReceiver(QObject* parent)
    : QObject(parent), m_socket(nullptr) {
    m_serialized.reserve(m_recvSize);
    m_buffer.setBuffer(&m_serialized);
    m_buffer.open(QIODevice::WriteOnly);
}

void EEGDataReceiver::start(const QString& host, const quint16 port) {
    m_socket = new QTcpSocket;
    connect(m_socket, &QTcpSocket::connected, [=] {
        emit connected();
        emit logFetched(tr("Connected to DSI-Streamer."), LogEdit::Success);
    });
    connect(m_socket, &QTcpSocket::readyRead, [=] {
        QByteArray data = m_socket->read(m_recvSize);
        data = m_cache + data; // 先拼接缓存的数据
        parse(data);
    });
    connect(m_socket, &QTcpSocket::disconnected, [=] {
        emit disconnected();
        emit logFetched(tr("Disconnected from DSI-Streamer."), LogEdit::Warning);
    });
    connect(m_socket, &QTcpSocket::errorOccurred, this, &EEGDataReceiver::onErrorOccurred);
    m_socket->connectToHost(host, port);
}

void EEGDataReceiver::stop() const {
    if (!m_socket) return;

    switch (m_socket->state()) {
        case QTcpSocket::ConnectedState:
            m_socket->disconnectFromHost();
            break;
        case QTcpSocket::ConnectingState:
            m_socket->abort();
            break;
        default:
            break;
    }
}

void EEGDataReceiver::parse(const QByteArray& raw) {
    int i = 0;                   // 解析指针
    const size_t n = raw.size(); // 数据包字节数

    // 解析出的EEG数据包
    QList<EEGSensorData> parsedData;

    while (i + 12 < n) {                   // 至少首先能有头部的长度
        if (raw.sliced(i, 5) == m_token) { // 解析前5个字节是否为token标记
            const quint8 packetType = raw.at(i + 5);
            const quint16 packetLength = qFromBigEndian<quint16>(raw.sliced(i + 6, 2));
            // quint32 packetNumber = qFromBigEndian<quint32>(raw.sliced(i + 8, 4));

            // 检查数据包完整性
            if (i + 12 + packetLength > n) break;

            if (packetType == 1) { // EEG数据包
                // 数据点原始毫秒级时间戳，相当于从开始采集的流逝时间
                qint64 timestamp = qFromBigEndian<float>(raw.sliced(i + 12, 4)) * 1000;
                // quint8 dataCounter = raw.at(i + 16);
                QByteArray adcStatus = raw.sliced(i + 17, 6);

                // 校验通道数量
                if ((packetLength - 11) % 4 != 0) {
                    // 数据包不合法，跳过
                    qWarning() << "Invalid packet length:" << packetLength;
                    i += 12 + packetLength;
                    continue;
                }
                const int numChannels = (packetLength - 11) / 4 - 1; // 最后一个通道实际上是trigger

                if (timestamp > 0) {
                    // 更新第一个数据包的时间戳
                    if (m_status == Prepared) { // 上一次是event_code = 2, 说明当前是第一个数据包
                        m_startLocalTime = QDateTime::currentMSecsSinceEpoch();
                        m_startDeviceTime = timestamp;
                        m_status = Receiving; // 变为正接收状态，下次不用再更新起始时间
                    }
                    // 更新时间戳
                    timestamp = m_startLocalTime + (timestamp - m_startDeviceTime);

                    std::array<float, 24> channelData{};
                    for (int ch = 0; ch < numChannels; ++ch) {
                        const int inCh = chReorderMap[ch];
                        channelData[ch] = qFromBigEndian<float>(raw.sliced(i + 23 + inCh * 4, 4));
                    }

                    const float trigger = qFromBigEndian<float>(raw.sliced(i + 23 + numChannels * 4, 4));

                    parsedData.append(EEGSensorData(timestamp, adcStatus, channelData, trigger));
                }
            } else if (packetType == 5) { // EEG事件包
                const quint32 code = qFromBigEndian<quint32>(raw.sliced(i + 12, 4));
                // quint32 node = qFromBigEndian<quint32>(raw.sliced(i + 16, 4));

                // 事件数据包的数据戳直接以本机时间戳为准
                const qint64 timestamp = QDateTime::currentMSecsSinceEpoch();

                auto parseMsg = [&] {
                    const quint32 msg_length = qFromBigEndian<quint32>(raw.sliced(i + 20, 4));
                    return raw.sliced(i + 24, msg_length).trimmed();
                };

                switch (code) {
                    case 1: { // Greeting/Version
                        auto version = parseMsg();
                        QVariantMap message = {
                            {"version", version}
                        };
                        emit eventFetched(EEGEventData(timestamp, code, message));
                        emit logFetched(tr("DSI-Streamer version: %1").arg(version));
                        break;
                    }
                    case 9: { // Sensor Map
                        auto sensorMap = parseMsg();
                        QVariantMap message = {
                            {"sensorMap", sensorMap}
                        };
                        emit eventFetched(EEGEventData(timestamp, code, message));
                        emit logFetched(tr("Original Sensor Map: %1").arg(sensorMap));
                        break;
                    }
                    case 10: { // Data Rate, {'mains': 50, 'sample': 300}
                        QList<QByteArray> rateList = parseMsg().split(',');
                        int f_m = rateList[0].toInt();
                        int f_s = rateList[1].toInt();
                        QVariantMap message = {
                            {"mainsFreq", f_m},
                            {"sampleFreq", f_s}
                        };
                        emit eventFetched(EEGEventData(timestamp, code, message));
                        emit logFetched(tr("Mains Frequency: %1 Hz, Sample Frequency: %2 Hz").arg(f_m).arg(f_s));
                        break;
                    }
                    case 2: { // start
                        m_status = Prepared;
                        emit eventFetched(EEGEventData(timestamp, code, QVariantMap()));
                        emit logFetched(tr("Started receiving EEG from DSI-Streamer."));
                        break;
                    }
                    case 3: { // stop
                        m_status = Initial;
                        emit eventFetched(EEGEventData(timestamp, code, QVariantMap()));
                        emit logFetched(tr("Stopped receiving EEG from DSI-Streamer."));
                        break;
                    }
                    default:
                        qDebug() << "Reserved/unknown event code:" << code;
                        break;
                }
            } else {
                qDebug() << "Reserved/unknown packet type:" << packetType;
            }

            i += 12 + packetLength; // 移动到下一个数据包
        }
        // 如果不是token标记，则继续向后查找
        else i++;
    }

    if (i < n) {
        m_cache = raw.sliced(i); // 剩余数据缓存
    } else {
        m_cache.clear(); // 清空缓存
    }

    if (!parsedData.empty()) {
        m_serialized.clear();
        m_buffer.seek(0);
        msgpack::pack(m_buffer, parsedData);

        const size_t count = parsedData.length();
        emit dataFetched(m_serialized, n, count);
        emit logFetched(tr("Received %1 bytes of data, %2 EEG Data packets").arg(n).arg(count));
    }
}

void EEGDataReceiver::onErrorOccurred(const QAbstractSocket::SocketError error) {
    emit errorOccurred(m_socket->error(), m_socket->errorString());
    QString msg;
    switch (error) {
        case QAbstractSocket::ConnectionRefusedError:
            msg = tr("Connection refused. There may be no DSI-Streamer running on the specified IP and port.");
            break;
        case QAbstractSocket::RemoteHostClosedError:
            msg = tr("The remote host closed the connection.");
            break;
        case QAbstractSocket::HostNotFoundError:
            msg = tr("The host was not found. Please check the IP address.");
            break;
        case QAbstractSocket::SocketAccessError:
            msg = tr("The socket operation failed due to insufficient permissions.");
            break;
        case QAbstractSocket::SocketTimeoutError:
            msg = tr("The operation timed out.");
            break;
        case QAbstractSocket::NetworkError:
            msg = tr("A network error occurred accidentally.");
            break;
        default:
            msg = m_socket->errorString();
            break;
    }
    emit logFetched(msg, LogEdit::Error);
}
