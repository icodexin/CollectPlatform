#include "BandViewController.h"
#include <QThread>
#include <QDebug>

namespace {
    constexpr int k_sampleRate = 50; // 50 Hz
    constexpr int k_renderRate = 30; // 30 FPS
}

BandViewWorker::BandViewWorker(QObject* parent) : QObject(parent) {
}

void BandViewWorker::pushData(const WristbandPacket& data) {
    Q_ASSERT(thread() == QThread::currentThread());
    constexpr double ratio = static_cast<double>(k_sampleRate) / k_renderRate;
    double counter = 0.0;
    for (qsizetype i = 0; i < data.length(); i++) {
        counter += 1.0;
        if (counter >= ratio) {
            m_queue.enqueue(BandViewFrame(data, i));
            counter -= ratio;
        }
    }
}

void BandViewWorker::fetchNextFrame() {
    Q_ASSERT(thread() == QThread::currentThread());
    if (!m_queue.isEmpty()) {
        const BandViewFrame frame = m_queue.dequeue();
        emit frameReady(frame);
    }
}

BandViewController::BandViewController(QObject* parent) : QObject(parent) {
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

void BandViewController::pushData(const WristbandPacket& data) {
    if (m_worker)
        QMetaObject::invokeMethod(m_worker, &BandViewWorker::pushData, Qt::QueuedConnection, data);
    else
        qWarning() << "Worker is not running, data is discarded.";
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
    if (m_timer.isActive())
        m_timer.stop();
    m_thread->start();
    m_timer.start(1000 / k_renderRate);
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

void BandViewController::onTimeout() {
    if (m_worker)
        QMetaObject::invokeMethod(m_worker, &BandViewWorker::fetchNextFrame, Qt::QueuedConnection);
    else
        qWarning() << "Worker is not running, cannot fetch frame.";
}
