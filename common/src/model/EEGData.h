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
    static constexpr int channelCount = 24;
    using ChArray = std::array<float, channelCount>;

    QByteArray adcStatus;
    ChArray channelData{};
    float trigger{};

    EEGSensorData() = default;
    EEGSensorData(qint64 timestamp, const QByteArray& adcStatus, const ChArray& channelData, float trigger);
};

struct EEGEventData final : EEGData {
    quint64 code{};
    QVariantMap message;

    EEGEventData() = default;
    EEGEventData(qint64 timestamp, quint64 code, const QVariantMap& message);
};

QDebug operator<<(QDebug, const EEGSensorData&);
QDebug operator<<(QDebug, const EEGEventData&);


#endif //EEGDATA_H
