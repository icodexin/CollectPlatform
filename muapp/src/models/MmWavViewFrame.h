#pragma once

#include <QtCore/QMetaType>
#include <QtCore/QPointF>
#include <QtCore/QVariantMap>

class MmwavViewFrame
{
public:
    MmwavViewFrame() = default;
    MmwavViewFrame(qint64 timestamp,
                   double heartWave,
                   double breathWave,
                   double heartRate,
                   double respiratoryRate);

    QPointF heartWavePoint() const;
    QPointF breathWavePoint() const;

    double heartRate() const;
    double respiratoryRate() const;

    QVariantMap toVariantMap() const;

private:
    QPointF m_heartWavePoint;
    QPointF m_breathWavePoint;
    double m_heartRate = -1.0;
    double m_respiratoryRate = -1.0;
};

Q_DECLARE_METATYPE(MmwavViewFrame)