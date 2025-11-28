#include "WristbandData.h"
#include <QJsonObject>
#include <QJsonArray>

namespace {
    // 手环数据采样间隔为20毫秒, 即采样率50Hz
    constexpr int k_sampleIntervalMs = 20;
    // 自动注册元类型
    const int _ = [] {
        qRegisterMetaType<WristbandPacket>("WristbandPacket");
        return 0;
    }();
}

PulseWaveValue::PulseWaveValue(const qint64 r, const qreal f)
    : raw{r}, filtered{f} {
}

PulseWaveValue::PulseWaveValue(const QJsonObject& json)
    : raw{json["rawVal"].toInteger()},
      filtered{json["filtedVal"].toDouble()} {
}

AccValue::AccValue(const qreal x, const qreal y, const qreal z)
    : x{x}, y{y}, z{z} {
}

AccValue::AccValue(const QJsonObject& json)
    : x{json["x"].toDouble()},
      y{json["y"].toDouble()},
      z{json["z"].toDouble()} {
}

WristbandPacket::WristbandPacket(const qint64 timestamp, const HrValue hr, const QList<PulseWaveValue>& pulseWaves,
                                 const QList<GsrValue>& gsrs, const QList<AccValue>& accs)
    : m_timestamp{timestamp}, m_hr{hr}, m_pulseWaves{pulseWaves}, m_gsrs{gsrs}, m_accs{accs} {
}

WristbandPacket WristbandPacket::fromJsonObject(const QJsonObject& json) {
    WristbandPacket packet;
    // 解析时间戳
    const qint64 timestamp = json["timestamp"].toInteger();
    packet.m_timestamp = timestamp;
    // 解析心率
    packet.m_hr = json["ppg"].toDouble();
    // 解析脉搏波列表
    auto pulseWaveArr = json["pulseWaveDatas"].toArray();
    packet.m_pulseWaves.reserve(pulseWaveArr.size());
    for (const auto& wave: pulseWaveArr) {
        packet.m_pulseWaves.append(PulseWaveValue{wave.toObject()});
    }
    // 解析皮肤电反应列表
    auto gsrArr = json["gsrs"].toArray();
    packet.m_gsrs.reserve(gsrArr.size());
    for (const auto& gsr: gsrArr) {
        packet.m_gsrs.append(gsr.toDouble());
    }
    // 解析加速度列表
    auto accArr = json["accDatas"].toArray();
    packet.m_accs.reserve(accArr.size());
    for (const auto& acc: accArr) {
        packet.m_accs.append(AccValue{acc.toObject()});
    }
    Q_ASSERT(pulseWaveArr.size() == gsrArr.size() && pulseWaveArr.size() == accArr.size());
    return packet;
}

qint64 WristbandPacket::timestamp(const qsizetype index) const {
    return m_timestamp + index * k_sampleIntervalMs;
}

HrValue WristbandPacket::hr(const qsizetype index) const {
    Q_UNUSED(index)
    return m_hr;
}

PulseWaveValue WristbandPacket::pulseWave(const qsizetype index) const {
    return m_pulseWaves.value(index, PulseWaveValue{});
}

GsrValue WristbandPacket::gsr(const qsizetype index) const {
    return m_gsrs.value(index, 0.0);
}

AccValue WristbandPacket::acc(const qsizetype index) const {
    return m_accs.value(index, AccValue{});
}

QList<PulseWaveValue> WristbandPacket::pulseWaveList() const {
    return m_pulseWaves;
}

QList<GsrValue> WristbandPacket::gsrList() const {
    return m_gsrs;
}

QList<AccValue> WristbandPacket::accList() const {
    return m_accs;
}

qsizetype WristbandPacket::length() const {
    return m_pulseWaves.size();
}

QDebug operator<<(QDebug debug, const PulseWaveValue& v) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "{raw:" << v.raw << ", filtered:" << v.filtered << "}";
    return debug;
}

QDebug operator<<(QDebug debug, const AccValue& v) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "{x:" << v.x << ", y:" << v.y << ", z:" << v.z << "}";
    return debug;
}

QDebug operator<<(QDebug debug, const WristbandPacket& p) {
    QDebugStateSaver saver(debug);
    debug.nospace() << "WristbandPacket("
            << "timestamp=" << p.timestamp()
            << ", hr=" << p.hr()
            << ", length=" << p.length()
            << ", pulseWaves=[";
    for (int i = 0; i < p.pulseWaveList().size(); ++i) {
        if (i > 0) debug.nospace() << ", ";
        debug.nospace() << p.pulseWaveList().at(i);
    }
    debug.nospace() << "], gsrs=[";
    for (int i = 0; i < p.gsrList().size(); ++i) {
        if (i > 0) debug.nospace() << ", ";
        debug.nospace() << p.gsrList().at(i);
    }
    debug.nospace() << "], accs=[";
    for (int i = 0; i < p.accList().size(); ++i) {
        if (i > 0) debug.nospace() << ", ";
        debug.nospace() << p.accList().at(i);
    }
    debug.nospace() << "])";
    return debug;
}
