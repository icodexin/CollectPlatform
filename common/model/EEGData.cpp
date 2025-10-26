#include "EEGData.h"
#include <QDateTime>

EEGData::EEGData(const qint64 timestamp, const DataType type)
    : timestamp(timestamp), type(type) {
}

EEGSensorData::EEGSensorData(
    const qint64 timestamp, const QByteArray& adcStatus, const std::array<float, 24>& channelData, const float trigger)
    : EEGData(timestamp, SensorData), adcStatus(adcStatus), channelData(channelData), trigger(trigger) {
}

EEGEventData::EEGEventData(const qint64 timestamp, const quint64 code, const QVariantMap& message)
    : EEGData(timestamp, EventData), code(code), message(message) {
}

QDebug operator<<(QDebug debug, const EEGSensorData& data) {
    QDebugStateSaver saver(debug);

    QStringList channel;
    for (const auto& ch: data.channelData) {
        channel << QString::number(ch);
    }

    debug.nospace().noquote() << "EEGSensorData(timestamp="
            << QDateTime::fromMSecsSinceEpoch(data.timestamp).toString("yyyy-MM-dd hh:mm:ss.zzz") << ", "
            << "adcStatus=" << data.adcStatus.toUInt() << ", "
            << "channelData=[" << channel.join(", ") << "], "
            << "trigger=" << data.trigger
            << ')';

    return debug;
}

QDebug operator<<(QDebug debug, const EEGEventData& data) {
    QDebugStateSaver saver(debug);

    debug.nospace().noquote() << "EEGEventData(timestamp="
            << QDateTime::fromMSecsSinceEpoch(data.timestamp).toString("yyyy-MM-dd hh:mm:ss.zzz") << ", "
            << "code=" << data.code << ", "
            << "message=" << data.message
            << ')';

    return debug;
}
