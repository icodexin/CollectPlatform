#ifndef MINMAXQUEUE_H
#define MINMAXQUEUE_H

#include <deque>
#include <QObject>
#include <QPointF>
#include <QtQmlIntegration/qqmlintegration.h>

class MinMaxQueue : public QObject {
    Q_OBJECT
    Q_PROPERTY(qreal min READ min NOTIFY minChanged)
    Q_PROPERTY(qreal max READ max NOTIFY maxChanged)
    QML_ELEMENT

public:
    explicit MinMaxQueue(QObject* parent = nullptr) : QObject(parent) {
    }

    ~MinMaxQueue() override = default;

    qreal min() const;
    qreal max() const;

    Q_INVOKABLE void append(const QPointF&);
    Q_INVOKABLE void append(const QList<QPointF>&);
    Q_INVOKABLE void appendParallelPoints(const QList<QPointF>&);
    Q_INVOKABLE void appendParallelLines(const QList<QList<QPointF> >&);
    Q_INVOKABLE void removeUntil(qint64 timestamp);

signals:
    void minChanged();
    void maxChanged();
    void minMaxChanged();

private:
    std::deque<QPointF> m_minQueue; // 维护最小值（单调递增）
    std::deque<QPointF> m_maxQueue; // 维护最大值（单调递减）

    void appendPointImpl(const QPointF& point);
    void appendParallelPointsImpl(const QList<QPointF>& points);
};

#endif //MINMAXQUEUE_H
