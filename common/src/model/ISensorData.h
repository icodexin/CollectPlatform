#ifndef ISENSORDATA_H
#define ISENSORDATA_H
#include <QtCore/QByteArray>

struct ISensorSerializer;

struct ISensorData {
    virtual ~ISensorData() = default;
    virtual QByteArray serialize(const ISensorSerializer& serializer) const = 0;
    virtual QString type() const = 0;
};

#endif //ISENSORDATA_H
