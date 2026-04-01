#ifndef MMWAVVIEWCONTROLLER_H
#define MMWAVVIEWCONTROLLER_H

#include <QtQmlIntegration/qqmlintegration.h>
#include <QtCore/QJsonObject>
#include <QtCore/QObject>
#include <QtCore/QPointer>
#include <QtCore/QQueue>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtQml/QQmlParserStatus>

#include "models/MmwavViewFrame.h"

class MmwavViewWorker final : public QObject
{
    Q_OBJECT

public:
    explicit MmwavViewWorker(QObject* parent = nullptr);

    signals:
        void frameReady(const MmwavViewFrame& frame);
    void samplingRateUpdated(int samplingRate);

public slots:
    void pushData(const QJsonObject& data, const QString& studentId);
    void fetchNextFrame();
    void clear();

private:
    QQueue<MmwavViewFrame> m_queue;
};

class MmwavViewController : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit MmwavViewController(QObject* parent = nullptr);
    ~MmwavViewController() override;

    void classBegin() override;
    void componentComplete() override;

public slots:
    Q_INVOKABLE void pushMmwavData(const QJsonObject& data, const QString& studentId);
    Q_INVOKABLE void reset();

    signals:
        void frameUpdated(const QVariantMap& frame);
    void resetRequested();

    // 发给 worker 线程
    void pushDataToWorker(const QJsonObject& data, const QString& studentId);
    void fetchNextFrameFromWorker();
    void clearWorkerQueue();

private slots:
    void onFrameReady(const MmwavViewFrame& frame);
    void onSamplingRateUpdated(int samplingRate);

private:
    QPointer<MmwavViewWorker> m_worker;
    QThread m_workerThread;
    QTimer m_timer;
};

#endif // MMWAVVIEWCONTROLLER_H