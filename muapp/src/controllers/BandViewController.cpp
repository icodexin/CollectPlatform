#include "BandViewController.h"

#include <QtCore/QMetaType>
#include <QtCore/QVariantList>
#include <QDebug>
#include <QThread>

namespace {
    constexpr int kDefaultSampleRate = 30; // Hz
    constexpr int kRenderRate = 30;        // FPS
}

BandViewWorker::BandViewWorker(QObject* parent) : QObject(parent) {
}

void BandViewWorker::pushJsonData(const QVariantMap& data, const QString& studentId) {
    Q_ASSERT(thread() == QThread::currentThread());
    Q_UNUSED(studentId);

    const qint64 baseTimestamp = data.value("timestamp").toLongLong();

    double samplingRate = data.value("sampling_rate").toDouble();
    if (samplingRate <= 0.0) {
        samplingRate = kDefaultSampleRate;
    }

    const QVariantList ppgFiltered = data.value("ppg_filtered").toList();
    const QVariantList accelX = data.value("accel_x").toList();
    const QVariantList accelY = data.value("accel_y").toList();
    const QVariantList accelZ = data.value("accel_z").toList();

    // 可选字段，没有就默认 0
    const QVariantList hrList = data.value("hr").toList();
    const QVariantList gsrList = data.value("gsr").toList();

    int n = ppgFiltered.size();
    if (accelX.size() < n) n = accelX.size();
    if (accelY.size() < n) n = accelY.size();
    if (accelZ.size() < n) n = accelZ.size();

    if (n <= 0) {
        qWarning() << "[BandViewWorker] wristband json has no valid data";
        return;
    }

    const double ratio = samplingRate / static_cast<double>(kRenderRate);
    double counter = 0.0;
    const qint64 sampleIntervalMs = static_cast<qint64>(1000.0 / samplingRate);

    for (int i = 0; i < n; ++i) {
        counter += 1.0;
        if (counter < ratio) {
            continue;
        }
        counter -= ratio;

        const qint64 ts = baseTimestamp + i * sampleIntervalMs;

        const qreal pulseWave = ppgFiltered[i].toDouble();
        const qreal ax = accelX[i].toDouble();
        const qreal ay = accelY[i].toDouble();
        const qreal az = accelZ[i].toDouble();

        const qreal hr = (i < hrList.size()) ? hrList[i].toDouble() : 0.0;
        const qreal gsr = (i < gsrList.size()) ? gsrList[i].toDouble() : 0.0;

        m_queue.enqueue(BandViewFrame(
            ts,
            hr,
            pulseWave,
            gsr,
            ax,
            ay,
            az
        ));
    }
}

void BandViewWorker::fetchNextFrame() {
    Q_ASSERT(thread() == QThread::currentThread());

    if (!m_queue.isEmpty()) {
        const BandViewFrame frame = m_queue.dequeue();
        emit frameReady(frame);
    }
}

void BandViewWorker::reset() {
    Q_ASSERT(thread() == QThread::currentThread());
    m_queue.clear();
}

BandViewController::BandViewController(QObject* parent) : QObject(parent) {
    qRegisterMetaType<BandViewFrame>("BandViewFrame");
    connect(&m_timer, &QTimer::timeout, this, &BandViewController::onTimeout);
}

BandViewController::~BandViewController() {
    stopRendering();
}

void BandViewController::classBegin() {
}

void BandViewController::componentComplete() {
    startRendering();
}

void BandViewController::pushJsonData(const QVariantMap& data, const QString& studentId) {
    if (m_worker) {
        QMetaObject::invokeMethod(
            m_worker,
            "pushJsonData",
            Qt::QueuedConnection,
            Q_ARG(QVariantMap, data),
            Q_ARG(QString, studentId)
        );
    } else {
        qWarning() << "Worker is not running, json data is discarded.";
    }
}

void BandViewController::startRendering() {
    if (m_worker || m_thread) {
        qWarning() << "Already in rendering state.";
        return;
    }

    m_worker = new BandViewWorker();
    m_thread = new QThread(this);
    m_worker->moveToThread(m_thread);

    connect(m_worker, &BandViewWorker::frameReady, this, &BandViewController::frameUpdated);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    if (m_timer.isActive()) {
        m_timer.stop();
    }

    m_thread->start();
    m_timer.start(1000 / kRenderRate);
}

void BandViewController::stopRendering() {
    m_timer.stop();

    if (m_worker && m_thread) {
        m_thread->quit();
        m_thread->wait();
        m_worker = nullptr;
        m_thread = nullptr;
    } else {
        qWarning() << "Not in rendering state.";
    }
}

void BandViewController::reset() {
    if (m_timer.isActive()) {
        m_timer.stop();
    }

    if (m_worker) {
        QMetaObject::invokeMethod(
            m_worker,
            "reset",
            Qt::BlockingQueuedConnection
        );
    }

    emit resetRequested();

    m_timer.start(1000 / kRenderRate);
}

void BandViewController::onTimeout() {
    if (m_worker) {
        QMetaObject::invokeMethod(
            m_worker,
            "fetchNextFrame",
            Qt::QueuedConnection
        );
    } else {
        qWarning() << "Worker is not running, cannot fetch frame.";
    }
}