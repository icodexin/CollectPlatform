#ifndef WRISTBANDDATA_H
#define WRISTBANDDATA_H

#include <QObject>

#include "ISensorData.h"

using HrValue = qreal;
using GsrValue = qreal;

struct PulseWaveValue {
    qint64  raw = 0.0;
    qreal   filtered = 0.0;
    PulseWaveValue() = default;
    PulseWaveValue(qint64 r, qreal f);
    explicit PulseWaveValue(const QJsonObject& json);
};

struct AccValue {
    qreal x = 0.0;
    qreal y = 0.0;
    qreal z = 0.0;
    AccValue() = default;
    AccValue(qreal x, qreal y, qreal z);
    explicit AccValue(const QJsonObject& json);
};

class WristbandPacket: public ISensorData {
public:
    WristbandPacket() = default;
    ~WristbandPacket() override = default;
    WristbandPacket(const WristbandPacket&) = default;
    WristbandPacket& operator=(const WristbandPacket&) = default;
    WristbandPacket(WristbandPacket&& other) noexcept = default;
    WristbandPacket& operator=(WristbandPacket&& other) noexcept = default;

    WristbandPacket(qint64 timestamp, HrValue hr, const QList<PulseWaveValue>& pulseWaves,
                    const QList<GsrValue>& gsrs, const QList<AccValue>& accs);
    static WristbandPacket fromJsonObject(const QJsonObject& json);

    QByteArray serialize(const ISensorSerializer& serializer) const override;
    QString type() const override;

    qint64 timestamp(qsizetype index = 0) const;
    HrValue hr(qsizetype index = 0) const;
    PulseWaveValue pulseWave(qsizetype index = 0) const;
    GsrValue gsr(qsizetype index = 0) const;
    AccValue acc(qsizetype index = 0) const;
    QList<PulseWaveValue> pulseWaveList() const;
    QList<GsrValue> gsrList() const;
    QList<AccValue> accList() const;

    qsizetype length() const;

private:
    qint64                  m_timestamp{};  // 毫秒时间戳
    HrValue                 m_hr{};         // 心率
    QList<PulseWaveValue>   m_pulseWaves;   // 脉搏波列表
    QList<GsrValue>         m_gsrs;         // 皮肤电反应列表
    QList<AccValue>         m_accs;         // 加速度列表
};

Q_DECLARE_METATYPE(WristbandPacket)

QDebug operator<<(QDebug, const PulseWaveValue&);
QDebug operator<<(QDebug, const AccValue&);
QDebug operator<<(QDebug, const WristbandPacket&);

#endif //WRISTBANDDATA_H
