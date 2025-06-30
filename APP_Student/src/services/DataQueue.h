//
// Created by Lenovo on 25-5-3.
//

#ifndef DATAQUEUE_H
#define DATAQUEUE_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QWaitCondition>

class DataQueueBase : public QObject {
    Q_OBJECT
signals:
    void dataAvailable();
};

template<typename T>
class DataQueue : public DataQueueBase {
public:
    void enqueue(const T& data) {
        QMutexLocker locker(&m_mutex);
        m_queue.enqueue(data);
        m_cond.wakeOne(); // 通知消费者有新数据

        // 如果队列之前为空，发出数据可用信号
        emit dataAvailable();
        // if (m_queue.size() == 1) {
        //     emit dataAvailable();
        // }
    }

    void enqueue(T&& data) {
        QMutexLocker locker(&m_mutex);
        m_queue.enqueue(std::move(data));
        m_cond.wakeOne(); // 通知消费者有新数据

        // 如果队列之前为空，发出数据可用信号
        emit dataAvailable();
        // if (m_queue.size() == 1) {
        //     emit dataAvailable();
        // }
    }

    T dequeue() {
        QMutexLocker locker(&m_mutex);
        while (m_queue.isEmpty()) {
            m_cond.wait(&m_mutex); // 等待直到有数据可用
        }
        return m_queue.dequeue();
    }

    bool isEmpty() const {
        QMutexLocker locker(&m_mutex);
        return m_queue.isEmpty();
    }

private:
    QQueue<T> m_queue;
    mutable QMutex m_mutex;
    QWaitCondition m_cond;
};


#endif //DATAQUEUE_H
