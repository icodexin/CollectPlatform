#include "DataSerializer.h"
#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include "model/serialize.h"

namespace {
    // 预编码常量字符串
    QByteArray packString(const char* s) {
        QByteArray out;
        QBuffer buf(&out);
        buf.open(QIODevice::WriteOnly);

        msgpack::packer packer(buf);
        packer.pack(s);

        return out;
    }

    const QByteArray kPackedStudentIDKey      = packString("student_id");
    const QByteArray kPackedDataTypeKey       = packString("data_type");
    const QByteArray kPackedDataKey           = packString("data");
    const QByteArray kPackedDataTypeEEGSensor = packString("eeg_sensor");
    const QByteArray kPackedDataTypeEEGEvent  = packString("eeg_event");
    const QByteArray kPackedDataTypeEEG       = packString("eeg");
    const QByteArray kPackedDataTypeWristband = packString("wristband");
}

DataSerializer::DataSerializer() {
    setStudentID("unknown");
}

QByteArray DataSerializer::serialize(const EEGSensorData& data) const {
    QByteArray out;
    QBuffer buffer(&out);
    buffer.open(QIODevice::WriteOnly);

    msgpack::packer packer(buffer);
    packer.pack_map(3);

    buffer.write(kPackedStudentIDKey);
    buffer.write(packedStudentId());

    buffer.write(kPackedDataTypeKey);
    buffer.write(kPackedDataTypeEEGSensor);

    buffer.write(kPackedDataKey);
    packer.pack(data);

    return out;
}

QByteArray DataSerializer::serialize(const EEGEventData& data) const {
    QByteArray out;
    QBuffer buffer(&out);
    buffer.open(QIODevice::WriteOnly);

    msgpack::packer packer(buffer);
    packer.pack_map(3);

    buffer.write(kPackedStudentIDKey);
    buffer.write(packedStudentId());

    buffer.write(kPackedDataTypeKey);
    buffer.write(kPackedDataTypeEEGEvent);

    buffer.write(kPackedDataKey);
    packer.pack(data);

    return out;
}

QByteArray DataSerializer::serialize(const WristbandPacket& data) const {
    QByteArray out;
    QBuffer buffer(&out);
    buffer.open(QIODevice::WriteOnly);

    msgpack::packer packer(buffer);
    packer.pack_map(3);

    buffer.write(kPackedStudentIDKey);
    buffer.write(packedStudentId());

    buffer.write(kPackedDataTypeKey);
    buffer.write(kPackedDataTypeWristband);

    buffer.write(kPackedDataKey);
    packer.pack(data);

    return out;
}

QByteArray DataSerializer::serialize(const EEGPacket& data) const {
    QByteArray out;
    QBuffer buffer(&out);
    buffer.open(QIODevice::WriteOnly);

    msgpack::packer packer(buffer);
    packer.pack_map(3);

    buffer.write(kPackedStudentIDKey);
    buffer.write(packedStudentId());

    buffer.write(kPackedDataTypeKey);
    buffer.write(kPackedDataTypeEEG);

    buffer.write(kPackedDataKey);
    packer.pack(data);

    return out;
}

QString DataSerializer::studentID() const {
    QMutexLocker lock(&m_mutex);
    return m_studentId;
}

void DataSerializer::setStudentID(const QString& id) {
    QMutexLocker lock(&m_mutex);
    m_studentId = id;

    QByteArray out;
    QBuffer buffer(&out);
    buffer.open(QIODevice::WriteOnly);
    msgpack::pack(buffer, id);

    m_packedStudentId = std::move(out);
}

QByteArray DataSerializer::packedStudentId() const {
    QMutexLocker lock(&m_mutex);
    return m_packedStudentId;
}
