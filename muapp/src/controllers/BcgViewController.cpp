#include "BcgViewController.h"

#include <QtCore/QJsonArray>
#include <QtCore/QThread>
#include <QtCore/QVariant>
#include <QtCore/QDebug>

#include <algorithm>

namespace {
    constexpr int k_defaultSampleRate = 30; // 默认采样率
    constexpr int k_renderRate = 30;         // 30 FPS
}

BcgViewWorker::BcgViewWorker(QObject* parent) : QObject(parent) {
}

void BcgViewWorker::reset()
{
    m_queue.clear();
}

void BcgViewWorker::pushData(const QJsonObject& data) {
    Q_ASSERT(thread() == QThread::currentThread());

    const QJsonArray heartWave = data.value("heart_waveform").toArray();
    const QJsonArray respWave  = data.value("resp_waveform").toArray();

    if (heartWave.isEmpty() || respWave.isEmpty()) {
        qWarning() << "BCG waveform is empty.";
        return;
    }

    const qsizetype frameCount = std::min(heartWave.size(), respWave.size());
    const int sampleRate = data.value("sampling_rate").toInt(k_defaultSampleRate);
    const qint64 startTimestamp = data.value("start_timestamp").toVariant().toLongLong();

    // 这两个值来自服务端
    const double heartRate = data.value("heart_rate").toDouble(-1.0);
    const double respiratoryRate = data.value("respiratory_rate").toDouble(-1.0);

    if (sampleRate <= 0) {
        qWarning() << "Invalid BCG sampling_rate:" << sampleRate;
        return;
    }

    const double ratio = static_cast<double>(sampleRate) / k_renderRate;
    double counter = 0.0;

    for (qsizetype i = 0; i < frameCount; ++i) {
        counter += 1.0;
        if (counter >= ratio) {
            const double heart = heartWave.at(i).toDouble();
            const double resp = respWave.at(i).toDouble();
            const qint64 timestamp = startTimestamp + static_cast<qint64>(1000.0 * i / sampleRate);

            m_queue.enqueue(BcgViewFrame(
                timestamp,
                heart,
                resp,
                heartRate,
                respiratoryRate
            ));
            counter -= ratio;
        }
    }
}

void BcgViewWorker::fetchNextFrame() {
    Q_ASSERT(thread() == QThread::currentThread());

    if (!m_queue.isEmpty()) {
        const BcgViewFrame frame = m_queue.dequeue();
        emit frameReady(frame);
    }
}

BcgViewController::BcgViewController(QObject* parent) : QObject(parent) {
    qRegisterMetaType<BcgViewFrame>("BcgViewFrame");
    connect(&m_timer, &QTimer::timeout, this, &BcgViewController::onTimeout);
}

BcgViewController::~BcgViewController() {
    stopRendering();
}

void BcgViewController::classBegin() {
}

void BcgViewController::componentComplete() {
    startRendering();
}

void BcgViewController::pushData(const QJsonObject& data, const QString& studentId) {
    Q_UNUSED(studentId)

    if (m_worker) {
        QMetaObject::invokeMethod(
            m_worker,
            &BcgViewWorker::pushData,
            Qt::QueuedConnection,
            data
        );
    } else {
        qWarning() << "Worker is not running, BCG data is discarded.";
    }
}

void BcgViewController::startRendering() {
    if (m_worker || m_thread) {
        qWarning() << "Already in rendering state.";
        return;
    }

    m_worker = new BcgViewWorker();
    m_thread = new QThread(this);
    m_worker->moveToThread(m_thread);

    connect(m_worker, &BcgViewWorker::frameReady, this, &BcgViewController::frameUpdated);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);

    if (m_timer.isActive())
        m_timer.stop();

    m_thread->start();
    m_timer.start(1000 / k_renderRate);
}

void BcgViewController::stopRendering() {
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

void BcgViewController::onTimeout() {
    if (m_worker) {
        QMetaObject::invokeMethod(
            m_worker,
            &BcgViewWorker::fetchNextFrame,
            Qt::QueuedConnection
        );
    } else {
        qWarning() << "Worker is not running, cannot fetch BCG frame.";
    }
}

void BcgViewController::reset()
{
    if (m_timer.isActive())
        m_timer.stop();

    if (m_worker) {
        QMetaObject::invokeMethod(
            m_worker,
            &BcgViewWorker::reset,
            Qt::BlockingQueuedConnection
        );
    }

    emit resetRequested();

    m_timer.start(1000 / k_renderRate);
}