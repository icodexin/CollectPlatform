#include "concurrency/QThreadPoolTaskExecutor.h"

#include <utility>

#include <QtCore/QRunnable>
#include <QtCore/QThreadPool>

QThreadPoolTaskExecutor::QThreadPoolTaskExecutor(QObject* parent)
    : ITaskExecutor(parent),
      m_threadPool(new QThreadPool(this)) {
}

QThreadPoolTaskExecutor::QThreadPoolTaskExecutor(QThreadPool* threadPool, QObject* parent)
    : ITaskExecutor(parent),
      m_threadPool(threadPool ? threadPool : QThreadPool::globalInstance()) {
}

void QThreadPoolTaskExecutor::submit(Task task) {
    if (!task || !m_threadPool) {
        return;
    }

    m_threadPool->start(QRunnable::create([task = std::move(task)]() mutable {
        task();
    }));
}

QThreadPool* QThreadPoolTaskExecutor::threadPool() const {
    return m_threadPool;
}

void QThreadPoolTaskExecutor::setMaxThreadCount(const int maxThreadCount) const {
    if (m_threadPool) {
        m_threadPool->setMaxThreadCount(maxThreadCount);
    }
}

int QThreadPoolTaskExecutor::maxThreadCount() const {
    return m_threadPool ? m_threadPool->maxThreadCount() : 0;
}
