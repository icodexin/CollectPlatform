#ifndef DATASERIALIZER_H
#define DATASERIALIZER_H

#include <QtCore/QMutex>

#include "model/ISensorSerializer.h"

class DataSerializer final : public ISensorSerializer {
public:
    DataSerializer();

    QByteArray serialize(const EEGSensorData& data) const override;
    QByteArray serialize(const EEGEventData& data) const override;
    QByteArray serialize(const WristbandPacket& data) const override;
    QByteArray serialize(const EEGPacket& data) const override;

    QString studentID() const;
    void setStudentID(const QString& id);

private:
    QByteArray packedStudentId() const;

private:
    QString m_studentId;
    QByteArray m_packedStudentId;
    mutable QMutex m_mutex;
};

#endif //DATASERIALIZER_H
