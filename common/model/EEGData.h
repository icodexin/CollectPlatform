#ifndef EEGDATA_H
#define EEGDATA_H

#include <array>
#include <QDebug>

struct EEGData {
    enum DataType {
        Unknown, SensorData, EventData
    };

    qint64 timestamp{};
    DataType type = Unknown;

    EEGData() = default;
    virtual ~EEGData() = default;

    explicit EEGData(qint64 timestamp, DataType type = Unknown);
};

struct EEGSensorData final : EEGData {
    QByteArray adcStatus;
    std::array<float, 24> channelData{};
    float trigger{};

    EEGSensorData() = default;

    EEGSensorData(qint64 timestamp, const QByteArray& adcStatus, const std::array<float, 24>& channelData, float trigger);
};

struct EEGEventData final : EEGData {
    quint64 code{};
    QVariantMap message;

    EEGEventData() = default;

    EEGEventData(qint64 timestamp, quint64 code, const QVariantMap& message);
};

QDataStream& operator<<(QDataStream& out, const EEGSensorData& data);

QDebug operator<<(QDebug debug, const EEGSensorData& data);

QDataStream& operator<<(QDataStream& out, const EEGEventData& data);

QDebug operator<<(QDebug debug, const EEGEventData& data);


#endif //EEGDATA_H
