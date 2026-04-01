#ifndef BANDVIEWFRAME_H
#define BANDVIEWFRAME_H

#include <QtCore/QPointF>
#include <QtCore/qtypes.h>
#include <QtQml/qqmlregistration.h>
#include <QObject>

class BandViewFrame {
    Q_GADGET
    Q_PROPERTY(QPointF hrPoint READ hrPoint)
    Q_PROPERTY(QPointF pulseWavePoint READ pulseWavePoint)
    Q_PROPERTY(QPointF gsrPoint READ gsrPoint)
    Q_PROPERTY(QPointF accXPoint READ accXPoint)
    Q_PROPERTY(QPointF accYPoint READ accYPoint)
    Q_PROPERTY(QPointF accZPoint READ accZPoint)
    QML_ANONYMOUS

public:
    BandViewFrame() = default;
    ~BandViewFrame() = default;
    BandViewFrame(const BandViewFrame&) = default;
    BandViewFrame& operator=(const BandViewFrame&) = default;

    BandViewFrame(qint64 timestamp,
                  qreal hr,
                  qreal pulseWaveFiltered,
                  qreal gsr,
                  qreal accX,
                  qreal accY,
                  qreal accZ);

    QPointF hrPoint() const;
    QPointF pulseWavePoint() const;
    QPointF gsrPoint() const;
    QPointF accXPoint() const;
    QPointF accYPoint() const;
    QPointF accZPoint() const;

private:
    QPointF m_hrPoint;
    QPointF m_pulseWavePoint;
    QPointF m_gsrPoint;
    QPointF m_accXPoint;
    QPointF m_accYPoint;
    QPointF m_accZPoint;
};

Q_DECLARE_METATYPE(BandViewFrame)

#endif // BANDVIEWFRAME_H