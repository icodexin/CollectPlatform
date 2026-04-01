#include "BandViewFrame.h"

BandViewFrame::BandViewFrame(qint64 timestamp,
                             qreal hr,
                             qreal pulseWaveFiltered,
                             qreal gsr,
                             qreal accX,
                             qreal accY,
                             qreal accZ)
    : m_hrPoint{static_cast<qreal>(timestamp), hr},
      m_pulseWavePoint{static_cast<qreal>(timestamp), pulseWaveFiltered},
      m_gsrPoint{static_cast<qreal>(timestamp), gsr},
      m_accXPoint{static_cast<qreal>(timestamp), accX},
      m_accYPoint{static_cast<qreal>(timestamp), accY},
      m_accZPoint{static_cast<qreal>(timestamp), accZ} {
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