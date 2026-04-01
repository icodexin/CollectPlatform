#include "RppgViewFrame.h"

RppgViewFrame::RppgViewFrame(const qint64 timestamp,
                             const double heartWave,
                             const double respWave,
                             const double heartRate,
                             const double respiratoryRate)
    : m_heartWavePoint{static_cast<qreal>(timestamp), static_cast<qreal>(heartWave)}
, m_respWavePoint{static_cast<qreal>(timestamp), static_cast<qreal>(respWave)}
, m_heartRate(heartRate)
, m_respiratoryRate(respiratoryRate)
{
}

QPointF RppgViewFrame::heartWavePoint() const
{
    return m_heartWavePoint;
}

QPointF RppgViewFrame::respWavePoint() const
{
    return m_respWavePoint;
}

double RppgViewFrame::heartRate() const
{
    return m_heartRate;
}

double RppgViewFrame::respiratoryRate() const
{
    return m_respiratoryRate;
}

QVariantMap RppgViewFrame::toVariantMap() const
{
    QVariantMap map;
    map.insert("heartWavePoint", QVariant::fromValue(m_heartWavePoint));
    map.insert("respWavePoint", QVariant::fromValue(m_respWavePoint));
    map.insert("heartRate", m_heartRate);
    map.insert("respiratoryRate", m_respiratoryRate);
    return map;
}