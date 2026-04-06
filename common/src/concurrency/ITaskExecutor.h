#ifndef ITASKEXECUTOR_H
#define ITASKEXECUTOR_H

#include <functional>

#include <QtCore/QObject>

/// 短任务执行器
class ITaskExecutor : public QObject {
    Q_OBJECT

public:
    using Task = std::function<void()>;

    explicit ITaskExecutor(QObject* parent = nullptr)
        : QObject(parent) {
    }

    ~ITaskExecutor() override = default;

    virtual void submit(Task task) = 0;
};

#endif // ITASKEXECUTOR_H
