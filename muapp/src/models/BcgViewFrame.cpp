#include "BcgViewFrame.h"

BcgViewFrame::BcgViewFrame(const qint64 timestamp,
                           const double heartWave,
                           const double respWave,
                           const double heartRate,
                           const double respiratoryRate)
    : m_heartWavePoint{static_cast<qreal>(timestamp), static_cast<qreal>(heartWave)}
, m_respWavePoint{static_cast<qreal>(timestamp), static_cast<qreal>(respWave)}
, m_heartRate(heartRate)
, m_respiratoryRate(respiratoryRate) {
}

QPointF BcgViewFrame::heartWavePoint() const {
    return m_heartWavePoint;
}

QPointF BcgViewFrame::respWavePoint() const {
    return m_respWavePoint;
}

double BcgViewFrame::heartRate() const {
    return m_heartRate;
}

double BcgViewFrame::respiratoryRate() const {
    return m_respiratoryRate;
}