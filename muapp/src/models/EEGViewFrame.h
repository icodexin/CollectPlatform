#ifndef EEGVIEWFRAME_H
#define EEGVIEWFRAME_H

#include <QtCore/QPointF>
#include <QtQml/qqmlregistration.h>
#include "model/EEGData.h"

class EEGViewFrame {
    Q_GADGET
    Q_PROPERTY(int length READ length)
    QML_ANONYMOUS

public:
    EEGViewFrame() = default;
    ~EEGViewFrame() = default;
    EEGViewFrame(const EEGViewFrame&) = default;
    EEGViewFrame& operator=(const EEGViewFrame&) = default;

    explicit EEGViewFrame(const EEGSensorData& data);
    EEGViewFrame(const EEGPacket& packet, int index);

    int length() const;
    Q_INVOKABLE QPointF channelAt(int index) const;

private:
    QList<QPointF> m_channels;
};

#endif //EEGVIEWFRAME_H