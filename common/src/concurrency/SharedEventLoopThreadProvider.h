#ifndef SHAREDEVENTLOOPTHREADPROVIDER_H
#define SHAREDEVENTLOOPTHREADPROVIDER_H

#include <QtCore/QHash>
#include <QtCore/QMutex>
#include <QtCore/QVector>

#include "IEventLoopThreadProvider.h"

/// 共享式事件循环线程提供器
class SharedEventLoopThreadProvider final : public IEventLoopThreadProvider {
    Q_OBJECT

public:
    explicit SharedEventLoopThreadProvider(qsizetype threadCount = 0, QObject* parent = nullptr);
    ~SharedEventLoopThreadProvider() override;

    QThread* acquire(const QString& hint) override;
    void release(QThread* thread) override;

    qsizetype threadCount() const;

private:
    static qsizetype defaultThreadCount();
    qsizetype selectThreadIndex(const QString& hint) const;

private:
    mutable QMutex m_mutex;
    QVector<QThread*> m_threads;
    QHash<QThread*, qsizetype> m_refCounts;
};

#endif // SHAREDEVENTLOOPTHREADPROVIDER_H
