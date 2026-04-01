#include "controllers/MmwavViewController.h"

#include <QtCore/QDateTime>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonValue>
#include <QtCore/QMetaType>
#include <QtCore/QtGlobal>

namespace {

static qreal sampleAt(const QJsonArray& arr, int index)
{
    if (index < 0 || index >= arr.size()) {
        return 0.0;
    }

    const QJsonValue v = arr.at(index);
    return v.isDouble() ? static_cast<qreal>(v.toDouble()) : 0.0;
}

static qint64 readStartTimestamp(const QJsonObject& data)
{
    const QJsonValue value = data.value("start_timestamp");

    if (value.isDouble()) {
        return static_cast<qint64>(value.toDouble());
    }

    const QVariant variant = value.toVariant();
    const qint64 ts = variant.toLongLong();
    if (ts > 0) {
        return ts;
    }

    return QDateTime::currentMSecsSinceEpoch();
}

static bool readOccupancy(const QJsonObject& data)
{
    const QJsonValue value = data.value("occupancy");

    if (value.isBool()) {
        return value.toBool();
    }

    if (value.isDouble()) {
        return !qFuzzyIsNull(value.toDouble());
    }

    const QVariant variant = value.toVariant();
    if (variant.canConvert<bool>()) {
        return variant.toBool();
    }

    return false;
}

} // namespace

MmwavViewWorker::MmwavViewWorker(QObject* parent)
    : QObject(parent)
{
}

void MmwavViewWorker::pushData(const QJsonObject& data, const QString& studentId)
{
    Q_UNUSED(studentId);

    const double HEART_GAIN = 50000.0;
    const double BREATH_GAIN = 50000.0;

    const qreal FLAT_HEART_VALUE = 0.0;
    const qreal FLAT_BREATH_VALUE = 0.0;

    const bool occupied = readOccupancy(data);

    const int samplingRate = qMax(1, data.value("sampling_rate").toInt(20));
    emit samplingRateUpdated(samplingRate);

    const QJsonArray heartWave = data.value("heart_wave_filtered").toArray();
    const QJsonArray breathWave = data.value("breath_wave_filtered").toArray();
    qDebug() << "heartWave =" << heartWave;
    qDebug() << "breathWave =" << breathWave;
    const int frameCount = qMin(heartWave.size(), breathWave.size());

    const qint64 startTimestamp =
        (frameCount > 0) ? readStartTimestamp(data)
                         : QDateTime::currentMSecsSinceEpoch();

    const qint64 dtMs = qMax<qint64>(1, qRound64(1000.0 / samplingRate));

    // 只保留最新一包
    m_queue.clear();

    // 关键：无人 / 空数组 时，主动补一段 0 波形
    if (!occupied || frameCount <= 0) {
        for (int i = 0; i < samplingRate; ++i) {
            const qint64 timestamp = startTimestamp + static_cast<qint64>(i) * dtMs;
            m_queue.enqueue(MmwavViewFrame(
                timestamp,
                FLAT_HEART_VALUE,
                FLAT_BREATH_VALUE,
                -1.0,
                -1.0
            ));
        }
        return;
    }

    // 有人且有数据：正常画真实波形
    const double heartRate = data.value("heart_rate").toDouble(-1.0);
    const double respiratoryRate = data.value("respiratory_rate").toDouble(-1.0);

    const int outCount = qMin(frameCount, samplingRate);
    const double step = static_cast<double>(frameCount) / outCount;

    for (int i = 0; i < outCount; ++i) {
        const int srcIndex = qMin(frameCount - 1, static_cast<int>(i * step));
        const qint64 timestamp = startTimestamp + static_cast<qint64>(srcIndex) * dtMs;

        const qreal heartPoint = sampleAt(heartWave, srcIndex) * HEART_GAIN;
        const qreal breathPoint = sampleAt(breathWave, srcIndex) * BREATH_GAIN;

        m_queue.enqueue(MmwavViewFrame(
            timestamp,
            heartPoint,
            breathPoint,
            heartRate,
            respiratoryRate
        ));
    }
}

void MmwavViewWorker::fetchNextFrame()
{
    if (m_queue.isEmpty()) {
        return;
    }

    emit frameReady(m_queue.dequeue());
}

void MmwavViewWorker::clear()
{
    m_queue.clear();
}

MmwavViewController::MmwavViewController(QObject* parent)
    : QObject(parent)
    , m_worker(new MmwavViewWorker(this))
{
    qRegisterMetaType<MmwavViewFrame>("MmwavViewFrame");

    connect(&m_timer, &QTimer::timeout,
            m_worker, &MmwavViewWorker::fetchNextFrame);

    connect(m_worker, &MmwavViewWorker::frameReady,
            this, &MmwavViewController::onFrameReady);

    connect(m_worker, &MmwavViewWorker::samplingRateUpdated,
            this, &MmwavViewController::onSamplingRateUpdated);

    m_timer.setTimerType(Qt::PreciseTimer);
    m_timer.setInterval(50);
}

MmwavViewController::~MmwavViewController() = default;

void MmwavViewController::classBegin()
{
}

void MmwavViewController::componentComplete()
{
    if (!m_timer.isActive()) {
        m_timer.start();
    }
}

void MmwavViewController::pushMmwavData(const QJsonObject& data, const QString& studentId)
{
    if (!m_worker) {
        return;
    }

    m_worker->pushData(data, studentId);
}

void MmwavViewController::reset()
{
    if (m_worker) {
        m_worker->clear();
    }

    emit resetRequested();
}

void MmwavViewController::onFrameReady(const MmwavViewFrame& frame)
{
    emit frameUpdated(frame.toVariantMap());
}

void MmwavViewController::onSamplingRateUpdated(int samplingRate)
{
    const int intervalMs = qMax(1, 1000 / qMax(1, samplingRate));
    m_timer.setInterval(intervalMs);

    if (!m_timer.isActive()) {
        m_timer.start();
    }
}