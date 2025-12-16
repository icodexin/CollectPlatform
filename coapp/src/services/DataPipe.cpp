#include "DataPipe.h"

#include <QtCore/QThread>
#include "DataSerializer.h"

DataPipeThread::DataPipeThread(QObject* parent)
    : QThread(parent) {
}

DataPipeThread::~DataPipeThread() {
    stop();
    wait();
}

void DataPipeThread::stop() {
    m_queue.close();
}

void DataPipeThread::setStudentID(const QString& id) {
    m_serializer.setStudentID(id);
}

bool DataPipeThread::push(DataPtr ptr) {
    return m_queue.push(std::move(ptr));
}

void DataPipeThread::run() {
    while (true) {
        auto result = m_queue.pop();
        if (!result.has_value()) {
            if (m_queue.isClosed())
                break;
            continue;
        }
        const auto in_ptr = std::move(result.value());
        if (!in_ptr)
            continue;
        QString type = in_ptr->type();
        QByteArray out = in_ptr->serialize(m_serializer);
        emit dataReady(type, out);
    }
}

DataPipe::DataPipe(QObject* parent) : QObject(parent) {
    m_thread = new DataPipeThread(this);
    connect(m_thread, &DataPipeThread::dataReady, this, &DataPipe::dataReady, Qt::QueuedConnection);
    m_thread->start();
}

DataPipe::~DataPipe() {
    if (m_thread) {
        m_thread->stop();
        m_thread->wait();
    }
}

bool DataPipe::isPushAllowed() const {
    return m_pushEnabled.load(std::memory_order_relaxed);
}

void DataPipe::allowPush(const bool allow) {
    m_pushEnabled.store(allow, std::memory_order_relaxed);
}

bool DataPipe::push(DataPtr ptr) {
    if (!ptr || !m_pushEnabled.load(std::memory_order_relaxed))
        return false;
    return m_thread->push(std::move(ptr));
}

void DataPipe::setStudentId(const QString& id) {
    if (!isPushAllowed()) {
        m_thread->setStudentID(id);
    }
}
