#include "concurrency/SharedEventLoopThreadProvider.h"

#include <limits>

#include <QtCore/QThread>

namespace {
    QString buildThreadName(const qsizetype index) {
        return QStringLiteral("SharedEventLoopThread/%1").arg(index);
    }
}

SharedEventLoopThreadProvider::SharedEventLoopThreadProvider(const qsizetype threadCount, QObject* parent)
    : IEventLoopThreadProvider(parent) {
    const qsizetype actualThreadCount = threadCount > 0 ? threadCount : defaultThreadCount();
    m_threads.reserve(actualThreadCount);

    // 一次性创建所有线程
    for (qsizetype i = 0; i < actualThreadCount; ++i) {
        auto* thread = new QThread(this);
        thread->setObjectName(buildThreadName(i));
        thread->start();

        m_threads.push_back(thread);
        m_refCounts.insert(thread, 0);
    }
}

SharedEventLoopThreadProvider::~SharedEventLoopThreadProvider() {
    // 统一销毁所有线程
    for (QThread* thread : m_threads) {
        if (!thread) {
            continue;
        }

        if (thread->isRunning()) {
            thread->quit();
            if (thread != QThread::currentThread()) {
                thread->wait();
            }
        }

        if (thread == QThread::currentThread()) {
            thread->deleteLater();
        } else {
            delete thread;
        }
    }
}

QThread* SharedEventLoopThreadProvider::acquire(const QString& hint) {
    QMutexLocker locker(&m_mutex);
    if (m_threads.isEmpty()) {
        return nullptr;
    }

    QThread* thread = m_threads.at(selectThreadIndex(hint));
    // 增加引用计数
    ++m_refCounts[thread];
    return thread;
}

void SharedEventLoopThreadProvider::release(QThread* thread) {
    if (!thread) {
        return;
    }

    QMutexLocker locker(&m_mutex);
    auto it = m_refCounts.find(thread);
    if (it == m_refCounts.end() || it.value() == 0) {
        return;
    }

    // 减少引用计数
    --it.value();
}

qsizetype SharedEventLoopThreadProvider::threadCount() const {
    QMutexLocker locker(&m_mutex);
    return m_threads.size();
}

qsizetype SharedEventLoopThreadProvider::defaultThreadCount() {
    return qMax<qsizetype>(1, QThread::idealThreadCount());
}

qsizetype SharedEventLoopThreadProvider::selectThreadIndex(const QString& hint) const {
    if (!hint.isEmpty()) {
        // 按 hint 哈希到固定线程
        return static_cast<qsizetype>(qHash(hint) % m_threads.size());
    }

    // 找到当前"借用次数"最少的线程
    qsizetype bestIndex = 0;
    qsizetype bestLoad = std::numeric_limits<qsizetype>::max();
    for (qsizetype i = 0; i < m_threads.size(); ++i) {
        const auto load = m_refCounts.value(m_threads.at(i), 0);
        if (load < bestLoad) {
            bestLoad = load;
            bestIndex = i;
        }
    }
    return bestIndex;
}
