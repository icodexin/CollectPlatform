#include "models/MmwavViewFrame.h"

MmwavViewFrame::MmwavViewFrame(const qint64 timestamp,
                               const double heartWave,
                               const double breathWave,
                               const double heartRate,
                               const double respiratoryRate)
    : m_heartWavePoint{static_cast<qreal>(timestamp), static_cast<qreal>(heartWave)}
, m_breathWavePoint{static_cast<qreal>(timestamp), static_cast<qreal>(breathWave)}
, m_heartRate(heartRate)
, m_respiratoryRate(respiratoryRate)
{
}

QPointF MmwavViewFrame::heartWavePoint() const
{
    return m_heartWavePoint;
}

QPointF MmwavViewFrame::breathWavePoint() const
{
    return m_breathWavePoint;
}

double MmwavViewFrame::heartRate() const
{
    return m_heartRate;
}

double MmwavViewFrame::respiratoryRate() const
{
    return m_respiratoryRate;
}

QVariantMap MmwavViewFrame::toVariantMap() const
{
    QVariantMap map;
    map.insert("heartWavePoint", QVariant::fromValue(m_heartWavePoint));
    map.insert("breathWavePoint", QVariant::fromValue(m_breathWavePoint));
    map.insert("heartRate", m_heartRate);
    map.insert("respiratoryRate", m_respiratoryRate);
    return map;
}