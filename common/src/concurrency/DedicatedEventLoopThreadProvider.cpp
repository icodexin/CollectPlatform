#include "concurrency/DedicatedEventLoopThreadProvider.h"

#include <QtCore/QThread>

DedicatedEventLoopThreadProvider::DedicatedEventLoopThreadProvider(QObject* parent)
    : IEventLoopThreadProvider(parent) {
}

DedicatedEventLoopThreadProvider::~DedicatedEventLoopThreadProvider() {
    QSet<QThread*> threads;
    {
        QMutexLocker locker(&m_mutex);
        threads = m_threads;
        m_threads.clear();
    }

    // 销毁所有线程
    for (QThread* thread : threads) {
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

QThread* DedicatedEventLoopThreadProvider::acquire(const QString& hint) {
    auto* thread = new QThread(this);
    if (!hint.isEmpty()) {
        thread->setObjectName(hint);
    }
    thread->start();

    QMutexLocker locker(&m_mutex);
    m_threads.insert(thread);
    return thread;
}

void DedicatedEventLoopThreadProvider::release(QThread* thread) {
    if (!thread) {
        return;
    }

    {
        QMutexLocker locker(&m_mutex);
        if (!m_threads.remove(thread)) {
            return;
        }
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

qsizetype DedicatedEventLoopThreadProvider::activeThreadCount() const {
    QMutexLocker locker(&m_mutex);
    return m_threads.size();
}
