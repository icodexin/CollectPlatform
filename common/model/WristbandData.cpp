//
// Created by Lenovo on 25-6-26.
//

#include "WristbandData.h"
#include <QMetaType>
#include <QJsonArray>
#include <QJsonObject>

namespace {
// 自动注册 WristbandData 类型到 Qt 元对象系统
const int _ = [] {
    qRegisterMetaType<PulseWaveData>("PulseWaveData");
    qRegisterMetaType<AccData>("AccData");
    qRegisterMetaType<WristbandData>("WristbandData");
    qRegisterMetaType<QList<PulseWaveData> >("QList<PulseWaveData>");
    qRegisterMetaType<QList<AccData> >("QList<AccData>");
    return 0;
}();
}

PulseWaveData PulseWaveData::fromJsonObject(const QJsonObject& json) {
    PulseWaveData data;
    data.raw = json["rawVal"].toDouble();
    data.filtered = json["filtedVal"].toDouble();
    return data;
}

AccData AccData::fromJsonObject(const QJsonObject& json) {
    AccData data;
    data.x = json["x"].toDouble();
    data.y = json["y"].toDouble();
    data.z = json["z"].toDouble();
    return data;
}

WristbandData WristbandData::fromJsonObject(const QJsonObject& json) {
    WristbandData data;
    // 解析时间戳
    data.m_timestamp = json["timestamp"].toInteger();
    // 解析心率
    data.m_hr = json["ppg"].toDouble();
    // 解析脉搏波数据
    auto pulseWaveArray = json["pulseWaveDatas"].toArray();
    data.m_pulseWaves.reserve(pulseWaveArray.size());
    for (const auto& item: pulseWaveArray) {
        QJsonObject obj = item.toObject();
        data.m_pulseWaves.append(PulseWaveData::fromJsonObject(obj));
    }
    // 解析皮肤电反应数据
    auto gsrArray = json["gsrs"].toArray();
    data.m_gsrs.reserve(gsrArray.size());
    for (const auto& item: gsrArray) {
        data.m_gsrs.append(item.toDouble());
    }
    // 解析加速度数据
    auto accArray = json["accDatas"].toArray();
    data.m_accs.reserve(accArray.size());
    for (const auto& item: accArray) {
        QJsonObject obj = item.toObject();
        data.m_accs.append(AccData::fromJsonObject(obj));
    }
    return data;
}

qint64 WristbandData::timestamp() const {
    return m_timestamp;
}

double WristbandData::hr() const {
    return m_hr;
}

QList<PulseWaveData> WristbandData::pulseWaves() const {
    return m_pulseWaves;
}

QList<qreal> WristbandData::gsrs() const {
    return m_gsrs;
}

QList<AccData> WristbandData::accs() const {
    return m_accs;
}

void WristbandData::setTimestamp(qint64 m_timestamp) {
    this->m_timestamp = m_timestamp;
}

void WristbandData::setHr(qreal m_hr) {
    this->m_hr = m_hr;
}

void WristbandData::setPulseWaves(const QList<PulseWaveData>& m_pulse_waves) {
    m_pulseWaves = m_pulse_waves;
}

void WristbandData::setGsrs(const QList<qreal>& m_gsrs) {
    this->m_gsrs = m_gsrs;
}

void WristbandData::setAccs(const QList<AccData>& m_accs) {
    this->m_accs = m_accs;
}

QDataStream& operator<<(QDataStream& out, const PulseWaveData& data) {
    out << "PulseWave(raw=" << data.raw << ", filtered=" << data.filtered << ")";
    return out;
}

QDataStream& operator<<(QDataStream& out, const AccData& data) {
    out << "Acc(x=" << data.x << ", y=" << data.y << ", z=" << data.z << ")";
    return out;
}

QDataStream& operator<<(QDataStream& out, const WristbandData& data) {
    out << "WristbandData(timestamp="
            << QDateTime::fromMSecsSinceEpoch(data.timestamp()).toString("yyyy-MM-dd hh:mm:ss.zzz") << ", "
            << ", hr=" << data.hr()
            << ", pulseWaves=" << data.pulseWaves()
            << ", gsrs=" << data.gsrs()
            << ", accs=" << data.accs() << ")";
    return out;
}

QDebug operator<<(QDebug debug, const PulseWaveData& data) {
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << "(" << data.raw << ", " << data.filtered << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const AccData& data) {
    QDebugStateSaver saver(debug);
    debug.nospace().noquote() << "(" << data.x << ", " << data.y << ", " << data.z << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const WristbandData& data) {
    QDebugStateSaver saver(debug);

    debug.nospace().noquote() << "WristbandData(timestamp="
            << QDateTime::fromMSecsSinceEpoch(data.timestamp()).toString("yyyy-MM-dd hh:mm:ss.zzz") << ", "
            << "pulse wave=" << data.pulseWaves() << ", "
            << "hr=" << data.hr() << ", "
            << "gsr=" << data.gsrs() << ", "
            << "acc=" << data.accs() << ")";
    return debug;
}
