#include "EEGReceiver.h"
#include <QtEndian>
#include "services/SettingsManager.h"
#include "model/serialize.h"

namespace {
    constexpr int k_channels = EEGSensorData::channelCount;

    const auto chReorderMap = [] {
        // 原始数据的通道顺序
        const std::array<std::string, k_channels> inChannels = {
            "P3", "C3", "F3", "Fz", "F4", "C4", "P4", "Cz", "CM", "A1",
            "Fp1", "Fp2", "T3", "T5", "O1", "O2", "X3", "X2", "F7", "F8",
            "X1", "A2", "T6", "T4"
        };
        // 输出数据的通道顺序
        const std::array<std::string, k_channels> outChannels = {
            "Fp1", "Fp2", "F7", "F3", "Fz", "F4", "F8", "A1", "T3", "C3",
            "Cz", "C4", "T4", "A2", "T5", "P3", "P4", "T6", "O1", "O2",
            "CM", "X1", "X2", "X3"
        };
        // 生成重排序映射
        std::array<int, k_channels> map{};
        for (int i = 0; i < outChannels.size(); ++i) {
            const auto it = std::find(inChannels.begin(), inChannels.end(), outChannels[i]);
            if (it != inChannels.end())
                map[i] = std::distance(inChannels.begin(), it);
            else
                qWarning() << "Channel" << outChannels[i] << "not found in original order.";
        }
        return map;
    }();

    int getRawChannelIndex(const int reorderedIndex) {
        Q_ASSERT(reorderedIndex >= 0 && reorderedIndex < k_channels);
        return chReorderMap[reorderedIndex];
    }

    constexpr int k_recvSize = 6000; // 0.04s * 4 bytes * 25 channels * 300Hz * 5
}

EEGDataParser::EEGDataParser(QObject* parent) {
    m_serialized.reserve(k_recvSize);
    m_buffer.setBuffer(&m_serialized);
    m_buffer.open(QIODevice::WriteOnly);
}

void EEGDataParser::operator()(const QByteArray& raw) {
    return parse(raw);
}

void EEGDataParser::parse(const QByteArray& raw) {
    // 拼接缓存和新接收的数据
    const QByteArray data = m_cache + raw;

    int i = 0;                    // 解析指针
    const size_t n = data.size(); // 数据包字节数

    // 解析出的EEG数据包
    QList<EEGSensorData> parsedData;

    while (i + 12 < n) {                    // 至少需要12字节才能构成一个数据包头
        if (data.sliced(i, 5) == m_token) { // 解析前5个字节是否为token标记
            const quint8 packetType = data.at(i + 5);
            const quint16 packetLength = qFromBigEndian<quint16>(data.sliced(i + 6, 2));
            // const quint32 packetNumber = qFromBigEndian<quint32>(raw.sliced(i + 8, 4));

            // 检查数据包完整性
            if (i + 12 + packetLength > n) break;

            if (packetType == 1) { // EEG数据包
                // 数据点原始毫秒级时间戳，相当于从开始采集的流逝时间
                auto timestamp = static_cast<qint64>(qFromBigEndian<float>(data.sliced(i + 12, 4)) * 1000);
                // quint8 dataCounter = raw.at(i + 16);
                QByteArray adcStatus = data.sliced(i + 17, 6);

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
                        m_status = Receiving; // 变为正在接收状态，下次不用再更新起始时间
                    }
                    // 更新为本机时间戳
                    timestamp = m_startLocalTime + (timestamp - m_startDeviceTime);

                    EEGSensorData::ChArray channelData{};
                    for (int ch = 0; ch < numChannels; ++ch) {
                        const int rawCh = getRawChannelIndex(ch);
                        channelData[ch] = qFromBigEndian<float>(data.sliced(i + 23 + rawCh * 4, 4));
                    }

                    const auto trigger = qFromBigEndian<float>(data.sliced(i + 23 + numChannels * 4, 4));

                    parsedData.append(EEGSensorData(timestamp, adcStatus, channelData, trigger));
                }
            } else if (packetType == 5) { // EEG事件包
                const quint32 code = qFromBigEndian<quint32>(data.sliced(i + 12, 4));
                // const quint32 node = qFromBigEndian<quint32>(raw.sliced(i + 16, 4));

                // 事件数据包的数据戳直接以本机时间戳为准
                const qint64 timestamp = QDateTime::currentMSecsSinceEpoch();

                // 只有部分事件码携带消息内容
                auto parseMsg = [&] {
                    const quint32 msg_length = qFromBigEndian<quint32>(raw.sliced(i + 20, 4));
                    return raw.sliced(i + 24, msg_length).trimmed();
                };

                bool validCode = true;
                QVariantMap msg;
                switch (code) {
                    case 1: { // Greeting/Version
                        msg["version"] = parseMsg();
                        break;
                    }
                    case 9: { // Sensor Map
                        msg["sensorMap"] = parseMsg();
                        break;
                    }
                    case 10: { // Data Rate
                        QList<QByteArray> rateList = parseMsg().split(',');
                        msg["mainsFreq"] = rateList[0].toInt();
                        msg["sampleFreq"] = rateList[1].toInt();
                        break;
                    }
                    case 2: { // start
                        m_status = Prepared;
                        break;
                    }
                    case 3: { // stop
                        m_status = Initial;
                        break;
                    }
                    default:
                        validCode = false;
                        qDebug() << "Reserved/unknown event code:" << code;
                        break;
                }

                if (validCode) {
                    emit eventParsed(EEGEventData(timestamp, code, msg));
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
        m_cache = data.sliced(i); // 剩余数据缓存
    } else {
        m_cache.clear(); // 清空缓存
    }

    if (!parsedData.empty()) {
        m_serialized.clear();
        m_buffer.seek(0);
        msgpack::pack(m_buffer, parsedData);

        const size_t count = parsedData.length();
        emit dataParsed(m_serialized, n, count);
    }
}

EEGReceiver::EEGReceiver(QObject* parent) : QObject(parent) {
    m_parser = new EEGDataParser(this);
    connect(m_parser, &EEGDataParser::dataParsed, this, &EEGReceiver::dataFetched);
    connect(m_parser, &EEGDataParser::eventParsed, this, &EEGReceiver::eventFetched);
}

void EEGReceiver::start(const QString& host, const quint16 port) {
    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, [=] {
        emit connected();
        emit logFetched(LogMessage::SUCCESS, tr("Connected to DSI-Streamer at %1:%2").arg(host).arg(port));
    });
    connect(m_socket, &QTcpSocket::disconnected, this, [=] {
        emit disconnected();
        emit logFetched(LogMessage::WARN, tr("Disconnected from DSI-Streamer at %1:%2").arg(host).arg(port));
    });
    connect(m_socket, &QTcpSocket::errorOccurred, this, &EEGReceiver::onErrorOccurred);
    connect(m_socket, &QTcpSocket::readyRead, this, [=] {
        const QByteArray raw = m_socket->read(k_recvSize);
        m_parser->parse(raw);
    });
    m_socket->connectToHost(host, port);
}

void EEGReceiver::stop() {
    if (!m_socket) return;

    disconnect(m_socket, &QTcpSocket::readyRead, nullptr, nullptr);

    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    } else {
        m_socket->abort();
    }

    m_socket->deleteLater();
    m_socket = nullptr;
}

void EEGReceiver::onErrorOccurred(const QAbstractSocket::SocketError error) {
    emit errorOccurred(error, m_socket->errorString());
    emit disconnected();
}
