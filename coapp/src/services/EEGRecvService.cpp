#include "EEGRecvService.h"

#include <algorithm>
#include <array>

#include <QtCore/QCoreApplication>
#include <QtCore/QDateTime>
#include <QtCore/QtEndian>
#include <QtNetwork/QTcpSocket>

#include "concurrency/DedicatedEventLoopThreadProvider.h"
#include "concurrency/EventLoopWorkerHost.h"
#include "model/serialize.h"

namespace {
    constexpr int k_channels = EEGSensorData::channelCount;
    constexpr int k_recvSize = 6000; // 0.04s * 4 bytes * 25 channels * 300Hz * 5
    constexpr auto k_eegPacketToken = "@ABCD";

    // 通道重排序映射
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

        // 生成重排序映射, map[新位置]->旧位置
        std::array<int, k_channels> map{};
        for (int i = 0; i < outChannels.size(); ++i) {
            const auto it = std::find(inChannels.begin(), inChannels.end(), outChannels[i]);
            if (it != inChannels.end()) {
                map[i] = static_cast<int>(std::distance(inChannels.begin(), it));
            } else {
                qWarning() << "Channel" << outChannels[i] << "not found in original order.";
            }
        }
        return map;
    }();

    int getRawChannelIndex(const int reorderedIndex) {
        Q_ASSERT(reorderedIndex >= 0 && reorderedIndex < k_channels);
        return chReorderMap[reorderedIndex];
    }
}

/******************** EEGDataParser  ********************/

EEGParseResult EEGDataParser::parse(const QByteArray& raw) {
    // 拼接缓存和新接收的数据
    const QByteArray data = m_cache + raw;

    int i = 0;                    // 解析指针
    const size_t n = data.size(); // 数据包字节数

    // 解析出的EEG数据包
    EEGParseResult result;
    result.rawSize = raw.size();

    while (i + 12 < n) {                                    // 至少需要12字节才能构成一个数据包头
        if (data.sliced(i, 5) == k_eegPacketToken) { // 解析前5个字节是否为token标记
            const quint8 packetType = data.at(i + 5);
            const quint16 packetLength = qFromBigEndian<quint16>(data.sliced(i + 6, 2));
            // const quint32 packetNumber = qFromBigEndian<quint32>(raw.sliced(i + 8, 4));

            // 检查数据包完整性
            if (i + 12 + packetLength > n) {
                break;
            }

            // EEG数据包
            if (packetType == 1) {
                // 数据点原始毫秒级时间戳，相当于从设备开始采集的流逝时间
                auto timestamp = static_cast<qint64>(qFromBigEndian<float>(data.sliced(i + 12, 4)) * 1000);
                // quint8 dataCounter = raw.at(i + 16);
                const QByteArray adcStatus = data.sliced(i + 17, 6);

                // 校验通道数量
                if ((packetLength - 11) % 4 != 0) {
                    qWarning() << "Invalid packet length:" << packetLength;
                    i += 12 + packetLength;
                    continue;
                }

                const int numChannels = (packetLength - 11) / 4 - 1; // 最后一个通道实际上是trigger

                if (timestamp > 0) {
                    // 更新第一个数据包的时间戳
                    if (m_status == Prepared && !m_hasTimeBase) { // 上一次是event_code = 2, 说明当前是第一个数据包
                        m_startLocalTime = QDateTime::currentMSecsSinceEpoch();
                        m_startDeviceTime = timestamp;
                        m_hasTimeBase = true;
                        m_status = Receiving; // 变为正在接收状态，下次不用再更新起始时间
                    }

                    if (!m_hasTimeBase) {
                        i += 12 + packetLength;
                        continue;
                    }

                    // 更新为本机时间戳
                    timestamp = m_startLocalTime + (timestamp - m_startDeviceTime);

                    EEGSensorData::ChArray channelData{};
                    for (int ch = 0; ch < numChannels; ++ch) {
                        const int rawCh = getRawChannelIndex(ch);
                        channelData[ch] = qFromBigEndian<float>(data.sliced(i + 23 + rawCh * 4, 4));
                    }

                    const auto trigger = qFromBigEndian<float>(data.sliced(i + 23 + numChannels * 4, 4));
                    result.data.append(EEGSensorData(timestamp, adcStatus, channelData, trigger));
                }
            }
            // EEG事件包
            else if (packetType == 5) {
                const quint32 code = qFromBigEndian<quint32>(data.sliced(i + 12, 4));
                // const quint32 node = qFromBigEndian<quint32>(raw.sliced(i + 16, 4));

                // 事件数据包的数据戳直接以本机时间戳为准
                const qint64 timestamp = QDateTime::currentMSecsSinceEpoch();

                // 只有部分事件码携带消息内容
                auto parseMsg = [&] {
                    const quint32 msgLength = qFromBigEndian<quint32>(data.sliced(i + 20, 4));
                    return data.sliced(i + 24, msgLength).trimmed();
                };

                bool validCode = true;
                QVariantMap msg;
                switch (code) {
                    case 1:  // Greeting/Version
                        msg["version"] = parseMsg();
                        break;
                    case 9:  // Sensor Map
                        msg["sensorMap"] = parseMsg();
                        break;
                    case 10: {  // Data Rate
                        const QList<QByteArray> rateList = parseMsg().split(',');
                        msg["mainsFreq"] = rateList.value(0).toInt();
                        msg["sampleFreq"] = rateList.value(1).toInt();
                        break;
                    }
                    case 2: // start
                        m_status = Prepared;
                        m_hasTimeBase = false;
                        break;
                    case 3: // stop
                        m_status = Initial;
                        m_hasTimeBase = false;
                        break;
                    default:
                        validCode = false;
                        qDebug() << "Reserved/unknown event code:" << code;
                        break;
                }

                if (validCode) {
                    result.events.append(EEGEventData(timestamp, code, msg));
                }
            }
            else {
                qDebug() << "Reserved/unknown packet type:" << packetType;
            }

            // 移动到下一个数据包
            i += 12 + packetLength;
        } else {
            // 如果不是token标记，则继续向后查找
            ++i;
        }
    }

    if (i < n) {
        // 剩余数据缓存
        m_cache = data.sliced(i);
    } else {
        // 清空缓存
        m_cache.clear();
    }
    return result;
}

void EEGDataParser::reset() {
    m_cache.clear();
    m_startLocalTime = 0;
    m_startDeviceTime = 0;
    m_hasTimeBase = false;
    m_status = Initial;
}

/******************** EEGRecvWorker  ********************/

EEGRecvWorker::EEGRecvWorker(QObject* parent)
    : QObject(parent) {
}

void EEGRecvWorker::onDataFetched(EEGPacketHandler callback) {
    m_dataHandler = std::move(callback);
}

void EEGRecvWorker::start(const QString& host, const quint16 port) {
    stop();

    m_host = host;
    m_port = port;
    m_parser.reset();

    m_socket = new QTcpSocket(this);
    connect(m_socket, &QTcpSocket::connected, this, &EEGRecvWorker::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &EEGRecvWorker::onDisconnected);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &EEGRecvWorker::onErrorOccurred);
    connect(m_socket, &QTcpSocket::readyRead, this, &EEGRecvWorker::onReadyRead);
    m_socket->connectToHost(host, port);
}

void EEGRecvWorker::stop() {
    if (!m_socket) {
        m_parser.reset();
        return;
    }

    const bool shouldNotify = m_socket->state() != QAbstractSocket::UnconnectedState;
    cleanupSocket();
    m_parser.reset();

    if (shouldNotify) {
        emit disconnected();
        emit logFetched(
            LogMessage::WARN,
            tr("Disconnected from DSI-Streamer at %1:%2").arg(m_host).arg(m_port)
        );
    }
}

void EEGRecvWorker::onConnected() {
    emit connected();
    emit logFetched(LogMessage::SUCCESS, tr("Connected to DSI-Streamer at %1:%2").arg(m_host).arg(m_port));
}

void EEGRecvWorker::onDisconnected() {
    cleanupSocket();
    m_parser.reset();
    emit disconnected();
    emit logFetched(LogMessage::WARN, tr("Disconnected from DSI-Streamer at %1:%2").arg(m_host).arg(m_port));
}

void EEGRecvWorker::onErrorOccurred(const QAbstractSocket::SocketError error) {
    const QString errorString = m_socket ? m_socket->errorString() : QString{};
    cleanupSocket();
    m_parser.reset();
    emit errorOccurred(error, errorString);
    emit disconnected();
}

void EEGRecvWorker::onReadyRead() {
    if (!m_socket) {
        return;
    }

    while (m_socket->bytesAvailable() > 0) {
        const QByteArray raw = m_socket->read(k_recvSize);
        if (raw.isEmpty()) {
            break;
        }

        EEGParseResult result = m_parser.parse(raw);

        for (const auto& event : result.events) {
            emit eventFetched(event);
        }

        if (result.hasData()) {
            emit dataFetched(result.rawSize, result.data.size());
            if (m_dataHandler) {
                auto packet = std::make_unique<EEGPacket>();
                packet->data = std::move(result.data);
                m_dataHandler(std::move(packet));
            }
        }
    }
}

void EEGRecvWorker::cleanupSocket() {
    if (!m_socket) {
        return;
    }

    m_socket->disconnect(this); // disconnect QT signals
    m_socket->abort();
    delete m_socket;
    m_socket = nullptr;
}

/******************** EEGRecvService  ********************/

EEGRecvService::EEGRecvService(QObject* parent)
    : EEGRecvService(new DedicatedEventLoopThreadProvider, true, parent) {
}

EEGRecvService::EEGRecvService(IEventLoopThreadProvider* threadProvider, QObject* parent)
    : EEGRecvService(threadProvider, false, parent) {
}

EEGRecvService::EEGRecvService(IEventLoopThreadProvider* threadProvider, const bool ownsThreadProvider, QObject* parent)
    : QObject(parent),
      EventLoopWorkerHost(threadProvider),
      m_ownsThreadProvider(ownsThreadProvider) {

    if (m_ownsThreadProvider) {
        threadProvider->setParent(this);
    }
}

EEGRecvService::~EEGRecvService() {
    if (hasWorker()) {
        call([worker = this->worker()] {
            worker->stop();
        });
    }
}

void EEGRecvService::onDataFetched(EEGPacketHandler callback) {
    m_dataFetchedCallback = std::move(callback);

    if (!hasWorker()) {
        return;
    }

    post([worker = this->worker(), callback = m_dataFetchedCallback] {
        worker->onDataFetched(callback);
    });
}

void EEGRecvService::start(const QString& host, const quint16 port) {
    ensureWorkerCreated();

    post([worker = this->worker(), host, port] {
        worker->start(host, port);
    });
}

void EEGRecvService::stop() {
    if (!hasWorker()) {
        return;
    }

    post([worker = this->worker()] {
        worker->stop();
    });
}

void EEGRecvService::ensureWorkerCreated() {
    if (hasWorker()) {
        return;
    }

    auto* worker = EventLoopWorkerHost::ensureWorkerCreated("EEGRecvService/Worker");
    bindWorkerSignals(worker);

    if (m_dataFetchedCallback) {
        post([worker, callback = m_dataFetchedCallback] {
            worker->onDataFetched(callback);
        });
    }
}

void EEGRecvService::bindWorkerSignals(EEGRecvWorker* worker) {
    connect(worker, &EEGRecvWorker::connected, this, &EEGRecvService::connected);
    connect(worker, &EEGRecvWorker::disconnected, this, &EEGRecvService::disconnected);
    connect(worker, &EEGRecvWorker::errorOccurred, this, &EEGRecvService::errorOccurred);
    connect(worker, &EEGRecvWorker::dataFetched, this, &EEGRecvService::dataFetched);
    connect(worker, &EEGRecvWorker::eventFetched, this, &EEGRecvService::eventFetched);
    connect(worker, &EEGRecvWorker::logFetched, this, &EEGRecvService::logFetched);
}
