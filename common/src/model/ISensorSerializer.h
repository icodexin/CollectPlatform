#ifndef ISENSORSERIALIZER_H
#define ISENSORSERIALIZER_H

#include <EEGData.h>
#include <WristbandData.h>

struct ISensorSerializer {
    virtual ~ISensorSerializer() = default;

    virtual QByteArray serialize(const EEGSensorData& data) const = 0;
    virtual QByteArray serialize(const EEGEventData& data) const = 0;
    virtual QByteArray serialize(const EEGPacket& dara) const = 0;
    virtual QByteArray serialize(const WristbandPacket& data) const = 0;
};

#endif //ISENSORSERIALIZER_H
