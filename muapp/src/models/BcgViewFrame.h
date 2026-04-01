#pragma once

#include <QtCore/QPointF>
#include <QtCore/QMetaType>
#include <QtCore/QObject>

class BcgViewFrame {
    Q_GADGET
    Q_PROPERTY(QPointF heartWavePoint READ heartWavePoint)
    Q_PROPERTY(QPointF respWavePoint READ respWavePoint)
    Q_PROPERTY(double heartRate READ heartRate)
    Q_PROPERTY(double respiratoryRate READ respiratoryRate)

public:
    BcgViewFrame() = default;
    BcgViewFrame(qint64 timestamp,
                 double heartWave,
                 double respWave,
                 double heartRate,
                 double respiratoryRate);

    QPointF heartWavePoint() const;
    QPointF respWavePoint() const;

    double heartRate() const;
    double respiratoryRate() const;

private:
    QPointF m_heartWavePoint;
    QPointF m_respWavePoint;
    double m_heartRate = -1.0;
    double m_respiratoryRate = -1.0;
};

Q_DECLARE_METATYPE(BcgViewFrame)