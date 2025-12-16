#ifndef EEGDATA_H
#define EEGDATA_H

#include <array>
#include <QtCore/QDebug>
#include "ISensorData.h"

struct EEGData: ISensorData {
    qint64 timestamp{};

    EEGData() = default;

    explicit EEGData(qint64 timestamp);
};

struct EEGSensorData final : EEGData {
    static constexpr int channelCount = 24;
    using ChArray = std::array<float, channelCount>;

    QByteArray adcStatus;
    ChArray channelData{};
    float trigger{};

    EEGSensorData() = default;
    EEGSensorData(qint64 timestamp, QByteArray adcStatus, const ChArray& channelData, float trigger);

    QByteArray serialize(const ISensorSerializer& serializer) const override;
    QString type() const override;
};
Q_DECLARE_METATYPE(EEGSensorData)

struct EEGEventData final : EEGData {
    quint64 code{};
    QVariantMap message;

    EEGEventData() = default;
    EEGEventData(qint64 timestamp, quint64 code, QVariantMap  message);

    QByteArray serialize(const ISensorSerializer& serializer) const override;
    QString type() const override;
};

struct EEGPacket final: ISensorData {
    QList<EEGSensorData> data;

    qsizetype length() const;
    EEGSensorData at(qsizetype index) const;

    QByteArray serialize(const ISensorSerializer& serializer) const override;
    QString type() const override;
};
Q_DECLARE_METATYPE(EEGPacket)

QDebug operator<<(QDebug, const EEGSensorData&);
QDebug operator<<(QDebug, const EEGEventData&);


#endif //EEGDATA_H
