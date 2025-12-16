#include "EEGViewController.h"

#include <QThread>

namespace {
    constexpr int k_sampleRate = 300; // 300 Hz
    constexpr int k_renderRate = 30;  // 30 FPS
}

EEGViewWorker::EEGViewWorker(QObject* parent) : QObject(parent) {
}

void EEGViewWorker::pushData(const EEGPacket& data) {
    Q_ASSERT(thread() == QThread::currentThread());
    constexpr double ratio = static_cast<double>(k_sampleRate) / k_renderRate;
    double counter = 0.0;
    for (qsizetype i = 0; i < data.length(); i++) {
        counter += 1.0;
        if (counter >= ratio) {
            m_queue.enqueue(EEGViewFrame(data, i));
            counter -= ratio;
        }
    }
}

void EEGViewWorker::fetchNextFrame() {
    Q_ASSERT(thread() == QThread::currentThread());
    if (!m_queue.isEmpty()) {
        const EEGViewFrame frame = m_queue.dequeue();
        emit frameReady(frame);
    }
}

EEGViewController::EEGViewController(QObject* parent): QObject(parent) {
    connect(&m_timer, &QTimer::timeout, this, &EEGViewController::onTimeout);
}

EEGViewController::~EEGViewController() {
    stopRendering();
}

void EEGViewController::classBegin() {
}

void EEGViewController::componentComplete() {
    startRendering();
}

void EEGViewController::pushData(const EEGPacket& data) {
    if (m_worker)
        QMetaObject::invokeMethod(m_worker, &EEGViewWorker::pushData, Qt::QueuedConnection, data);
    else
        qWarning() << "Worker is not running, data is discarded.";
}

void EEGViewController::startRendering() {
    if (m_worker || m_thread) {
        qWarning() << "Already in rendering state.";
        return;
    }
    m_worker = new EEGViewWorker();
    m_thread = new QThread(this);
    m_worker->moveToThread(m_thread);
    connect(m_worker, &EEGViewWorker::frameReady, this, &EEGViewController::frameUpdated);
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QObject::deleteLater);
    if (m_timer.isActive())
        m_timer.stop();
    m_thread->start();
    m_timer.start(1000 / k_renderRate);
}

void EEGViewController::stopRendering() {
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

void EEGViewController::onTimeout() {
    if (m_worker)
        QMetaObject::invokeMethod(m_worker, &EEGViewWorker::fetchNextFrame, Qt::QueuedConnection);
    else
        qWarning() << "Worker is not running, cannot fetch frame.";
}
