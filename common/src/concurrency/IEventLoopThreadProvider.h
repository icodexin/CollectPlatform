#ifndef IEVENTLOOPTHREADPROVIDER_H
#define IEVENTLOOPTHREADPROVIDER_H

#include <QtCore/QObject>
#include <QtCore/QString>

class QThread;

/// 事件循环线程上下文提供器接口, 统一管理/复用线程资源
class IEventLoopThreadProvider : public QObject {
    Q_OBJECT

public:
    explicit IEventLoopThreadProvider(QObject* parent = nullptr)
        : QObject(parent) {
    }

    ~IEventLoopThreadProvider() override = default;

    /// 借出线程
    virtual QThread* acquire(const QString& hint) = 0;
    /// 归还线程
    virtual void release(QThread* thread) = 0;
};

#endif // IEVENTLOOPTHREADPROVIDER_H
