#ifndef WRISTBANDDATA_H
#define WRISTBANDDATA_H

#include <QtTypes>
#include <QVector>
#include <QDateTime>
#include <QDebug>
#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <qqmlintegration.h>

class WristbandData {
    Q_GADGET
    Q_PROPERTY(qint64 timestamp READ timestamp)
    Q_PROPERTY(qreal hr READ hr)
    Q_PROPERTY(QVariantList pulseWaves READ pulseWaves)
    Q_PROPERTY(QVariantList gsrs READ gsrs)
    Q_PROPERTY(QVariantList accs READ accs)
    QML_VALUE_TYPE(wristbandData)

public:
    WristbandData() = default;

    static WristbandData fromJsonObject(const QJsonObject& json) {
        WristbandData data;

        // 解析时间戳
        data.m_timestamp = json["timestamp"].toInteger();

        // 解析心率
        data.m_hr = json["ppg"].toDouble();

        // 解析脉搏波数据
        data.m_pulseWaves = json["pulseWaveDatas"].toArray().toVariantList();

        // 解析皮肤电反应数据
        data.m_gsrs = json["gsrs"].toArray().toVariantList();

        // 解析加速度数据
        data.m_accs = json["accDatas"].toArray().toVariantList();

        return data;
    }

    qint64 timestamp() const { return m_timestamp; }

    qreal hr() const { return m_hr; }

    QVariantList pulseWaves() const { return m_pulseWaves; }

    QVariantList gsrs() const { return m_gsrs; }

    QVariantList accs() const { return m_accs; }

private:
    qint64 m_timestamp{};         // 毫秒级时间戳
    qreal m_hr{};                 // 心率
    QList<QVariant> m_pulseWaves; // 脉搏波数据
    QList<QVariant> m_gsrs;       // 皮肤电反应数据
    QList<QVariant> m_accs;       // 加速度数据
};

inline QDataStream& operator<<(QDataStream& out, const WristbandData& data) {
    out << "WristbandData(timestamp="
            << QDateTime::fromMSecsSinceEpoch(data.timestamp()).toString("yyyy-MM-dd hh:mm:ss.zzz") << ", "
            << "pulse wave=" << data.pulseWaves() << ", "
            << "hr=" << data.hr() << ", "
            << "gsr=" << data.gsrs() << ", "
            << "acc=" << data.accs() << ")";
    return out;
}

inline QDebug operator<<(QDebug debug, const WristbandData& data) {
    QDebugStateSaver saver(debug);

    QStringList pwdList;
    for (const auto& value: data.pulseWaves()) {
        auto obj = value.toJsonObject();
        pwdList << QString("(%1, %2)")
                .arg(obj["rawVal"].toDouble())
                .arg(obj["filtedVal"].toDouble());
    }

    QStringList gsrList;
    for (const auto& value: data.gsrs()) {
        gsrList << QString::number(value.toDouble());
    }

    QStringList accList;
    for (const auto& value: data.accs()) {
        auto obj = value.toJsonObject();
        accList << QString("(%1, %2, %3)")
                .arg(obj["x"].toDouble())
                .arg(obj["y"].toDouble())
                .arg(obj["z"].toDouble());
    }

    debug.nospace().noquote() << "WristbandData(timestamp="
            << QDateTime::fromMSecsSinceEpoch(data.timestamp()).toString("yyyy-MM-dd hh:mm:ss.zzz") << ", "
            << "pulse wave=[" << pwdList.join(", ") << "], "
            << "hr=" << data.hr() << ", "
            << "gsr=[" << gsrList.join(", ") << "], "
            << "acc=[" << accList.join(", ") << "])";
    return debug;
}


#endif //WRISTBANDDATA_H
