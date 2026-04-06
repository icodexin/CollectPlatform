#ifndef DEDICATEDEVENTLOOPTHREADPROVIDER_H
#define DEDICATEDEVENTLOOPTHREADPROVIDER_H

#include <QtCore/QMutex>
#include <QtCore/QSet>

#include "IEventLoopThreadProvider.h"

/// 独占式专用事件循环线程提供器
class DedicatedEventLoopThreadProvider final : public IEventLoopThreadProvider {
    Q_OBJECT

public:
    explicit DedicatedEventLoopThreadProvider(QObject* parent = nullptr);
    ~DedicatedEventLoopThreadProvider() override;

    /// 获取一个新线程, 并立即启动
    QThread* acquire(const QString& hint) override;
    /// 释放并销毁线程
    void release(QThread* thread) override;

    qsizetype activeThreadCount() const;

private:
    mutable QMutex m_mutex;
    QSet<QThread*> m_threads;
};

#endif // DEDICATEDEVENTLOOPTHREADPROVIDER_H
