#include "EEGData.h"
#include <QtCore/QDateTime>
#include <utility>

#include "ISensorSerializer.h"

namespace {
    const int _ = [] {
        qRegisterMetaType<EEGSensorData>("EEGSensorData");
        qRegisterMetaType<EEGPacket>("EEGPacket");
        return 0;
    }();
}

EEGData::EEGData(const qint64 timestamp)
    : timestamp(timestamp) {
}

EEGSensorData::EEGSensorData(
    const qint64 timestamp, QByteArray adcStatus, const std::array<float, 24>& channelData, const float trigger)
    : EEGData(timestamp), adcStatus(std::move(adcStatus)), channelData(channelData), trigger(trigger) {
}

QByteArray EEGSensorData::serialize(const ISensorSerializer& serializer) const {
    return serializer.serialize(*this);
}

QString EEGSensorData::type() const {
    return "eeg_sensor";
}

EEGEventData::EEGEventData(const qint64 timestamp, const quint64 code, QVariantMap message)
    : EEGData(timestamp), code(code), message(std::move(message)) {
}

QByteArray EEGEventData::serialize(const ISensorSerializer& serializer) const {
    return serializer.serialize(*this);
}

QString EEGEventData::type() const {
    return "eeg_event";
}

qsizetype EEGPacket::length() const {
    return data.size();
}

EEGSensorData EEGPacket::at(const qsizetype index) const {
    return data.at(index);
}

QByteArray EEGPacket::serialize(const ISensorSerializer& serializer) const {
    return serializer.serialize(*this);
}

QString EEGPacket::type() const {
    return "eeg";
}

QDebug operator<<(QDebug debug, const EEGSensorData& data) {
    QDebugStateSaver saver(debug);

    QStringList channel;
    for (const auto& ch : data.channelData) {
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
