#ifndef IMMEDIATETASKEXECUTOR_H
#define IMMEDIATETASKEXECUTOR_H

#include <utility>

#include "ITaskExecutor.h"

/// 同步立即式任务执行器
class ImmediateTaskExecutor final : public ITaskExecutor {
    Q_OBJECT

public:
    explicit ImmediateTaskExecutor(QObject* parent = nullptr)
        : ITaskExecutor(parent) {
    }

    void submit(Task task) override {
        if (task) {
            std::move(task)();
        }
    }
};

#endif // IMMEDIATETASKEXECUTOR_H
