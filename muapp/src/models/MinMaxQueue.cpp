#include "MinMaxQueue.h"
#include <QDebug>

namespace {
    class MinMaxGuard {
    public:
        explicit MinMaxGuard(MinMaxQueue* queue)
            : m_queue(queue), m_oldMin(queue->min()), m_oldMax(queue->max()) {
        }

        ~MinMaxGuard() {
            const qreal newMin = m_queue->min();
            const qreal newMax = m_queue->max();

            if (m_oldMin != newMin)
                emit m_queue->minChanged();
            if (m_oldMax != newMax)
                emit m_queue->maxChanged();
            if (m_oldMin != newMin || m_oldMax != newMax)
                emit m_queue->minMaxChanged();
        }

    private:
        MinMaxQueue* m_queue;
        qreal m_oldMin;
        qreal m_oldMax;
    };
}

qreal MinMaxQueue::min() const {
    return m_minQueue.empty() ? qQNaN() : m_minQueue.front().y();
}

qreal MinMaxQueue::max() const {
    return m_maxQueue.empty() ? qQNaN() : m_maxQueue.front().y();
}

void MinMaxQueue::append(const QPointF& point) {
    MinMaxGuard guard(this);
    appendPointImpl(point);
}

void MinMaxQueue::append(const QList<QPointF>& points) {
    if (points.isEmpty())
        return;

    MinMaxGuard guard(this);
    for (const QPointF& point: points)
        appendPointImpl(point);
}

void MinMaxQueue::appendParallelPoints(const QList<QPointF>& points) {
    MinMaxGuard guard(this);
    appendParallelPointsImpl(points);
}

void MinMaxQueue::appendParallelLines(const QList<QList<QPointF>>& pointsList) {
    if (pointsList.isEmpty())
        return;

    MinMaxGuard guard(this);
    const qsizetype times = pointsList[0].size();
#ifdef QT_DEBUG
    for (qsizetype i = 0; i < pointsList.size(); i++)
        Q_ASSERT(pointsList[i].size() == times); // 保证每个列表长度相同
#endif
    for (qsizetype j = 0; j < times; j++) {
        QList<QPointF> points; // 收集第j个时间点的所有点
        points.reserve(pointsList.size());
        for (qsizetype i = 0; i < pointsList.size(); i++)
            points.append(pointsList[i][j]);
        appendParallelPointsImpl(points);
    }
}

void MinMaxQueue::removeUntil(const qint64 timestamp) {
    MinMaxGuard guard(this);

    // 更新最大值队列
    while (!m_maxQueue.empty() && m_maxQueue.front().x() <= timestamp)
        m_maxQueue.pop_front();

    // 更新最小值队列
    while (!m_minQueue.empty() && m_minQueue.front().x() <= timestamp)
        m_minQueue.pop_front();
}

void MinMaxQueue::appendPointImpl(const QPointF& point) {
    // 维护最小值队列：移除尾部所有比新元素大的元素
    while (!m_minQueue.empty() && m_minQueue.back().y() > point.y())
        m_minQueue.pop_back();
    m_minQueue.push_back(point);

    // 维护最大值队列：移除尾部所有比新元素小的元素
    while (!m_maxQueue.empty() && m_maxQueue.back().y() < point.y())
        m_maxQueue.pop_back();
    m_maxQueue.push_back(point);
}

void MinMaxQueue::appendParallelPointsImpl(const QList<QPointF>& points) {
    if (points.isEmpty())
        return;

    const auto timestamp = points.front().x();
    auto minY = points.front().y();
    auto maxY = points.front().y();
    for (qsizetype i = 1; i < points.size(); ++i) {
        const auto& point = points[i];
        Q_ASSERT(point.x() == timestamp); // 保证时间戳相同
        if (point.y() < minY)
            minY = point.y();
        if (point.y() > maxY)
            maxY = point.y();
    }
    appendPointImpl({timestamp, minY});
    appendPointImpl({timestamp, maxY});
}