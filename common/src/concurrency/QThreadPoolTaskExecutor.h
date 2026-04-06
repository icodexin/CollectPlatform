#ifndef QTHREADPOOLTASKEXECUTOR_H
#define QTHREADPOOLTASKEXECUTOR_H

#include "ITaskExecutor.h"

class QThreadPool;

/// 基于 QThreadPool 的通用任务执行器
class QThreadPoolTaskExecutor final : public ITaskExecutor {
    Q_OBJECT

public:
    /// 构造独占私有线程池的任务执行器
    explicit QThreadPoolTaskExecutor(QObject* parent = nullptr);
    /// 构造复用外部线程池的任务执行器
    explicit QThreadPoolTaskExecutor(QThreadPool* threadPool, QObject* parent = nullptr);
    ~QThreadPoolTaskExecutor() override = default;

    void submit(Task task) override;

    QThreadPool* threadPool() const;
    /// 设置最大线程数
    void setMaxThreadCount(int maxThreadCount) const;
    int maxThreadCount() const;

private:
    QThreadPool* m_threadPool = nullptr;
};

#endif // QTHREADPOOLTASKEXECUTOR_H
