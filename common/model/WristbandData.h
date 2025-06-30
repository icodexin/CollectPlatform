//
// Created by Lenovo on 25-6-26.
//

#ifndef WRISTBANDDATA_H
#define WRISTBANDDATA_H

#include <QObject>

struct PulseWaveData {
    qint64 raw{0};       // 原始脉搏波数据
    qreal filtered{0.0}; // 过滤后的脉搏波数据

    static PulseWaveData fromJsonObject(const QJsonObject& json);
};

Q_DECLARE_METATYPE(PulseWaveData);

struct AccData {
    qreal x{0.0};
    qreal y{0.0};
    qreal z{0.0};

    static AccData fromJsonObject(const QJsonObject& json);
};

Q_DECLARE_METATYPE(AccData);

class WristbandData {
public:
    WristbandData() = default;

    static WristbandData fromJsonObject(const QJsonObject& json);

    qint64 timestamp() const;
    qreal hr() const;
    QList<PulseWaveData> pulseWaves() const;
    QList<qreal> gsrs() const;
    QList<AccData> accs() const;

    void setTimestamp(qint64 m_timestamp);
    void setHr(qreal m_hr);
    void setPulseWaves(const QList<PulseWaveData>& m_pulse_waves);
    void setGsrs(const QList<qreal>& m_gsrs);
    void setAccs(const QList<AccData>& m_accs);

private:
    qint64 m_timestamp{};              // 毫秒级时间戳
    qreal m_hr{};                      // 心率
    QList<PulseWaveData> m_pulseWaves; // 脉搏波数据
    QList<qreal> m_gsrs;               // 皮肤电反应数据
    QList<AccData> m_accs;             // 加速度数据
};

Q_DECLARE_METATYPE(WristbandData)


QDataStream& operator<<(QDataStream& out, const PulseWaveData& data);

QDataStream& operator<<(QDataStream& out, const AccData& data);

QDataStream& operator<<(QDataStream& out, const WristbandData& data);

QDebug operator<<(QDebug debug, const PulseWaveData& data);

QDebug operator<<(QDebug debug, const AccData& data);

QDebug operator<<(QDebug debug, const WristbandData& data);


#endif //WRISTBANDDATA_H
