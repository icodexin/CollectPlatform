#include "EEGViewFrame.h"

EEGViewFrame::EEGViewFrame(const EEGSensorData& data) {
    m_channels.reserve(data.channelCount);
    qreal timestamp = static_cast<qreal>(data.timestamp);
    for (const auto ch: data.channelData) {
        m_channels.append({
            timestamp, static_cast<qreal>(ch)
        });
    }
}

EEGViewFrame::EEGViewFrame(const EEGPacket& packet, const int index)
    : EEGViewFrame(packet.at(index)) {
}

int EEGViewFrame::length() const {
    return m_channels.size();
}

QPointF EEGViewFrame::channelAt(int index) const {
    return m_channels.at(index);
}
