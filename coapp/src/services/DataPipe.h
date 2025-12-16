#ifndef DATAPIPE_H
#define DATAPIPE_H

#include <atomic>
#include <QtCore/QThread>

#include "DataSerializer.h"
#include "model/CQueue.h"
#include "model/ISensorData.h"

class DataPipeThread final : public QThread {
    Q_OBJECT

public:
    using DataPtr = std::unique_ptr<ISensorData>;
    using DataQueue = CQueue<DataPtr>;

    explicit DataPipeThread(QObject* parent = nullptr);
    ~DataPipeThread() override;

    void stop();
    void setStudentID(const QString& id);

    bool push(DataPtr ptr);

signals:
    void dataReady(QString dataType, QByteArray data);

protected:
    void run() override;

private:
    DataQueue m_queue;
    DataSerializer m_serializer;
};

class DataPipe final : public QObject {
    Q_OBJECT

public:
    using DataPtr = std::unique_ptr<ISensorData>;
    using DataQueue = CQueue<DataPtr>;

    explicit DataPipe(QObject* parent = nullptr);
    ~DataPipe() override;

    bool isPushAllowed() const;
    void allowPush(bool allow);
    bool push(DataPtr ptr);

    // todo: 当前版本限制在push后不允许设置studentID, 后续接入登录系统后需继续考虑此处设计(或改为直接在原始数据中注入)
    void setStudentId(const QString& id);

signals:
    void dataReady(QString dataType, QByteArray data);

private:
    DataPipeThread* m_thread = nullptr;
    std::atomic<bool> m_pushEnabled = true;
};

#endif //DATAPIPE_H
