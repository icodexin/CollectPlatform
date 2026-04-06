#ifndef EVENTLOOPWORKERHOST_H
#define EVENTLOOPWORKERHOST_H

#include <functional>
#include <type_traits>
#include <utility>

#include <QtCore/QMetaObject>
#include <QtCore/QThread>

#include "IEventLoopThreadProvider.h"

/**
 * 事件循环线程宿主，托管后台线程Worker的生命周期与方法调用
 * @tparam Worker 线程工作者, 实现具体的后台逻辑
 */
template <typename Worker>
class EventLoopWorkerHost {
    static_assert(std::is_base_of_v<QObject, Worker>, "Worker must inherit QObject");

public:
    using WorkerFactory = std::function<Worker*()>;

    explicit EventLoopWorkerHost(IEventLoopThreadProvider* threadProvider, WorkerFactory factory = {})
        : m_threadProvider(threadProvider),
          m_factory(std::move(factory)) {
        Q_ASSERT(m_threadProvider);
    }

    ~EventLoopWorkerHost() {
        destroyWorker();
    }

    EventLoopWorkerHost(const EventLoopWorkerHost&) = delete;
    EventLoopWorkerHost& operator=(const EventLoopWorkerHost&) = delete;

    EventLoopWorkerHost(EventLoopWorkerHost&&) = delete;
    EventLoopWorkerHost& operator=(EventLoopWorkerHost&&) = delete;

    Worker* worker() const {
        return m_worker;
    }

    QThread* thread() const {
        return m_thread;
    }

    bool hasWorker() const {
        return m_worker != nullptr;
    }

    Worker* ensureWorkerCreated() {
        return ensureWorkerCreated(QString{});
    }

    Worker* ensureWorkerCreated(const QString& threadHint = {}) {
        if (m_worker) {
            return m_worker;
        }

        m_thread = m_threadProvider->acquire(threadHint);
        Q_ASSERT(m_thread);

        m_worker = m_factory ? m_factory() : new Worker;
        Q_ASSERT(m_worker);
        m_worker->moveToThread(m_thread);
        return m_worker;
    }

    /// 异步投递函数
    template <typename Functor>
    bool post(Functor&& functor) const {
        if (!m_worker) {
            return false;
        }

        return QMetaObject::invokeMethod(
            m_worker,
            std::forward<Functor>(functor),
            Qt::QueuedConnection
        );
    }

    /// 同步调用函数
    template <typename Functor>
    bool call(Functor&& functor) const {
        if (!m_worker) {
            return false;
        }

        if (QThread::currentThread() == m_worker->thread()) {
            std::forward<Functor>(functor)();
            return true;
        }

        return QMetaObject::invokeMethod(
            m_worker,
            std::forward<Functor>(functor),
            Qt::BlockingQueuedConnection
        );
    }

    void destroyWorker() {
        if (!m_worker) {
            return;
        }

        Worker* worker = m_worker;
        QThread* thread = m_thread;
        m_worker = nullptr;
        m_thread = nullptr;

        // 销毁工作者对象
        if (thread && thread->isRunning() && QThread::currentThread() != thread) {
            QMetaObject::invokeMethod(worker, [worker] { delete worker; }, Qt::BlockingQueuedConnection);
        } else {
            delete worker;
        }

        // 释放线程对象
        if (thread) {
            m_threadProvider->release(thread);
        }
    }

private:
    IEventLoopThreadProvider* m_threadProvider = nullptr;
    WorkerFactory m_factory;
    Worker* m_worker = nullptr;
    QThread* m_thread = nullptr;
};

#endif // EVENTLOOPWORKERHOST_H
