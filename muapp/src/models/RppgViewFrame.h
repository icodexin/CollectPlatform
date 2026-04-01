#pragma once

#include <QtCore/QMetaType>
#include <QtCore/QPointF>
#include <QtCore/QVariantMap>

class RppgViewFrame
{
public:
    RppgViewFrame() = default;
    RppgViewFrame(qint64 timestamp,
                  double heartWave,
                  double respWave,
                  double heartRate,
                  double respiratoryRate);

    QPointF heartWavePoint() const;
    QPointF respWavePoint() const;

    double heartRate() const;
    double respiratoryRate() const;

    QVariantMap toVariantMap() const;

private:
    QPointF m_heartWavePoint;
    QPointF m_respWavePoint;
    double m_heartRate = -1.0;
    double m_respiratoryRate = -1.0;
};

Q_DECLARE_METATYPE(RppgViewFrame)