#include "BandViewFrame.h"

BandViewFrame::BandViewFrame(const qint64 timestamp, const HrValue hr, const PulseWaveValue& pulseWave,
                             const GsrValue gsr, const AccValue& acc)
    : m_hrPoint{static_cast<qreal>(timestamp), hr},
      m_pulseWavePoint{static_cast<qreal>(timestamp), pulseWave.filtered},
      m_gsrPoint{static_cast<qreal>(timestamp), gsr},
      m_accXPoint{static_cast<qreal>(timestamp), acc.x},
      m_accYPoint{static_cast<qreal>(timestamp), acc.y},
      m_accZPoint{static_cast<qreal>(timestamp), acc.z} {
}

BandViewFrame::BandViewFrame(const WristbandPacket& packet, const qsizetype index)
    : BandViewFrame (
        packet.timestamp(index), packet.hr(index), packet.pulseWave(index),
        packet.gsr(index), packet.acc(index)
    ) {
}

QPointF BandViewFrame::hrPoint() const {
    return m_hrPoint;
}

QPointF BandViewFrame::pulseWavePoint() const {
    return m_pulseWavePoint;
}

QPointF BandViewFrame::gsrPoint() const {
    return m_gsrPoint;
}

QPointF BandViewFrame::accXPoint() const {
    return m_accXPoint;
}

QPointF BandViewFrame::accYPoint() const {
    return m_accYPoint;
}

QPointF BandViewFrame::accZPoint() const {
    return m_accZPoint;
}
