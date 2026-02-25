#include "VideoPullService.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

#include <QtCore/QLoggingCategory>
#include <QtCore/QMetaObject>
#include <QtCore/QTimer>

Q_LOGGING_CATEGORY(vpPull, "Services.VideoPullService")

namespace {
    // 直接映射到 Qt 格式（无需 SWS 转换）
    struct DirectMap {
        AVPixelFormat avFmt;
        QVideoFrameFormat::PixelFormat qtFmt;
        int planes;
    };

    constexpr DirectMap kDirectMap[] = {
        {AV_PIX_FMT_NV12, QVideoFrameFormat::Format_NV12, 2},
        {AV_PIX_FMT_NV21, QVideoFrameFormat::Format_NV21, 2},
        {AV_PIX_FMT_YUV420P, QVideoFrameFormat::Format_YUV420P, 3},
        {AV_PIX_FMT_YUYV422, QVideoFrameFormat::Format_YUYV, 1},
        {AV_PIX_FMT_UYVY422, QVideoFrameFormat::Format_UYVY, 1},
    };

    QVideoFrameFormat::ColorRange fromAvColorRange(const AVColorRange r) {
        switch (r) {
            case AVCOL_RANGE_JPEG: return QVideoFrameFormat::ColorRange_Full;
            case AVCOL_RANGE_MPEG: return QVideoFrameFormat::ColorRange_Video;
            default: return QVideoFrameFormat::ColorRange_Unknown;
        }
    }

    QVideoFrameFormat::ColorSpace fromAvColorSpace(const AVColorSpace cs) {
        switch (cs) {
            case AVCOL_SPC_BT709: return QVideoFrameFormat::ColorSpace_BT709;
            case AVCOL_SPC_BT470BG:
            case AVCOL_SPC_SMPTE170M: return QVideoFrameFormat::ColorSpace_BT601;
            case AVCOL_SPC_RGB: return QVideoFrameFormat::ColorSpace_AdobeRgb;
            case AVCOL_SPC_BT2020_CL:
            case AVCOL_SPC_BT2020_NCL: return QVideoFrameFormat::ColorSpace_BT2020;
            default: return QVideoFrameFormat::ColorSpace_Undefined;
        }
    }

    QVideoFrameFormat::ColorTransfer fromAvColorTrc(const AVColorTransferCharacteristic trc) {
        switch (trc) {
            case AVCOL_TRC_BT709: return QVideoFrameFormat::ColorTransfer_BT709;
            case AVCOL_TRC_SMPTE170M: return QVideoFrameFormat::ColorTransfer_BT601;
            case AVCOL_TRC_LINEAR: return QVideoFrameFormat::ColorTransfer_Linear;
            case AVCOL_TRC_GAMMA22: return QVideoFrameFormat::ColorTransfer_Gamma22;
            case AVCOL_TRC_GAMMA28: return QVideoFrameFormat::ColorTransfer_Gamma28;
            case AVCOL_TRC_SMPTE2084: return QVideoFrameFormat::ColorTransfer_ST2084;
            case AVCOL_TRC_ARIB_STD_B67: return QVideoFrameFormat::ColorTransfer_STD_B67;
            default: return QVideoFrameFormat::ColorTransfer_Unknown;
        }
    }

    /// 打印 FFmpeg 错误信息
    void logAvError(const char* prefix, int err) {
        char buf[AV_ERROR_MAX_STRING_SIZE] = {};
        av_strerror(err, buf, sizeof(buf));
        qCWarning(vpPull) << prefix << ":" << buf;
    }
} // namespace

VideoPullWorker::VideoPullWorker(QObject* parent)
    : QThread(parent)
{}

VideoPullWorker::~VideoPullWorker() {
    requestStop();
    wait(); // 确保线程退出后再析构
}

void VideoPullWorker::setUrl(const QString& url) {
    m_url = url;
}

void VideoPullWorker::requestStop() {
    m_stopRequested.storeRelaxed(1);
}

int VideoPullWorker::interruptCallback(void* ctx) {
    auto* self = static_cast<VideoPullWorker*>(ctx);
    return self->m_stopRequested.loadRelaxed() ? 1 : 0;
}

void VideoPullWorker::run() {
    m_stopRequested.storeRelaxed(0);
    emit statusChanged(VideoPullService::Connecting);

    // 1) 分配输入上下文，注册中断回调
    m_ifmt = avformat_alloc_context();
    if (!m_ifmt) {
        emit errorOccurred(-1, QStringLiteral("avformat_alloc_context failed"));
        emit statusChanged(VideoPullService::Error);
        return;
    }
    m_ifmt->interrupt_callback.callback = interruptCallback;
    m_ifmt->interrupt_callback.opaque = this;

    // 低延迟输入选项
    AVDictionary* inputOpts = nullptr;
    av_dict_set(&inputOpts, "rtsp_transport", "udp", 0);     // UDP 传输
    av_dict_set(&inputOpts, "fflags", "nobuffer", 0);        // 禁用输入缓冲
    av_dict_set(&inputOpts, "flags", "low_delay", 0);        // 低延迟标志
    av_dict_set(&inputOpts, "reorder_queue_size", "0", 0);   // 禁用抖动缓冲队列
    av_dict_set(&inputOpts, "max_delay", "0", 0);            // 最大延迟 0
    av_dict_set(&inputOpts, "analyzeduration", "100000", 0); // 探测时长 100ms
    av_dict_set(&inputOpts, "probesize", "131072", 0);       // 探测大小 128KB
    av_dict_set(&inputOpts, "stimeout", "5000000", 0);       // 连接超时 5s

    // 2) 打开输入流
    int err = avformat_open_input(
        &m_ifmt,
        m_url.toUtf8().constData(),
        nullptr,
        &inputOpts
    );
    av_dict_free(&inputOpts);

    if (err < 0 || m_stopRequested.loadRelaxed()) {
        if (err < 0) {
            logAvError("avformat_open_input", err);
            emit errorOccurred(err, QStringLiteral("Failed to open RTSP stream: ") + m_url);
        }
        emit statusChanged(VideoPullService::Error);
        cleanup();
        return;
    }

    // 3) 获取流信息（限制探测时间）
    AVDictionary* findOpts = nullptr;
    av_dict_set(&findOpts, "analyzeduration", "100000", 0);
    av_dict_set(&findOpts, "probesize", "131072", 0);
    err = avformat_find_stream_info(m_ifmt, &findOpts);
    av_dict_free(&findOpts);

    if (err < 0 || m_stopRequested.loadRelaxed()) {
        if (err < 0) {
            logAvError("avformat_find_stream_info", err);
            emit errorOccurred(err, QStringLiteral("Failed to find stream info"));
        }
        emit statusChanged(VideoPullService::Error);
        cleanup();
        return;
    }

    // 4) 查找最佳视频流
    const AVCodec* decoder = nullptr;
    m_vStreamIndex = av_find_best_stream(m_ifmt, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (m_vStreamIndex < 0 || !decoder) {
        emit errorOccurred(-1, QStringLiteral("No video stream found"));
        emit statusChanged(VideoPullService::Error);
        cleanup();
        return;
    }

    // 5) 初始化解码器
    m_dec = avcodec_alloc_context3(decoder);
    if (!m_dec) {
        emit errorOccurred(-1, QStringLiteral("avcodec_alloc_context3 failed"));
        emit statusChanged(VideoPullService::Error);
        cleanup();
        return;
    }

    err = avcodec_parameters_to_context(m_dec, m_ifmt->streams[m_vStreamIndex]->codecpar);
    if (err < 0) {
        logAvError("avcodec_parameters_to_context", err);
        emit errorOccurred(err, QStringLiteral("avcodec_parameters_to_context failed"));
        emit statusChanged(VideoPullService::Error);
        cleanup();
        return;
    }

    // 低延迟解码选项
    m_dec->flags |= AV_CODEC_FLAG_LOW_DELAY;
    m_dec->flags2 |= AV_CODEC_FLAG2_FAST;
    // 单线程解码延迟最低（多线程会引入帧缓冲）
    m_dec->thread_count = 1;
    m_dec->thread_type = 0;

    AVDictionary* decOpts = nullptr;
    av_dict_set(&decOpts, "flags", "low_delay", 0);
    av_dict_set(&decOpts, "fflags", "nobuffer", 0);
    err = avcodec_open2(m_dec, decoder, &decOpts);
    av_dict_free(&decOpts);

    if (err < 0) {
        logAvError("avcodec_open2", err);
        emit errorOccurred(err, QStringLiteral("Failed to open decoder"));
        emit statusChanged(VideoPullService::Error);
        cleanup();
        return;
    }

    // 6) 分配复用资源
    m_pkt = av_packet_alloc();
    m_frame = av_frame_alloc();
    if (!m_pkt || !m_frame) {
        emit errorOccurred(-1, QStringLiteral("av_packet_alloc / av_frame_alloc failed"));
        emit statusChanged(VideoPullService::Error);
        cleanup();
        return;
    }

    emit statusChanged(VideoPullService::Playing);
    qCInfo(vpPull) << "Decode loop started for" << m_url;

    // 7) 主解码循环
    while (!m_stopRequested.loadRelaxed()) {
        err = av_read_frame(m_ifmt, m_pkt);

        if (err == AVERROR(EAGAIN)) {
            // 当前无可用数据，短暂等待后重试
            av_packet_unref(m_pkt);
            QThread::usleep(500);
            continue;
        }
        if (err == AVERROR_EOF || err == AVERROR(EINTR)) {
            // 流结束或被中断
            av_packet_unref(m_pkt);
            break;
        }
        if (err < 0) {
            logAvError("av_read_frame", err);
            av_packet_unref(m_pkt);
            emit errorOccurred(err, QStringLiteral("av_read_frame error (connection lost?)"));
            emit statusChanged(VideoPullService::Error);
            cleanup();
            return;
        }

        // 只处理视频流
        if (m_pkt->stream_index != m_vStreamIndex) {
            av_packet_unref(m_pkt);
            continue;
        }

        // 送入解码器
        err = avcodec_send_packet(m_dec, m_pkt);
        av_packet_unref(m_pkt);
        if (err < 0 && err != AVERROR(EAGAIN) && err != AVERROR_EOF) {
            logAvError("avcodec_send_packet", err);
            continue;
        }

        // 取出解码帧（可能一次 packet 得到多帧）
        while (!m_stopRequested.loadRelaxed()) {
            err = avcodec_receive_frame(m_dec, m_frame);
            if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) {
                break;
            }
            if (err < 0) {
                logAvError("avcodec_receive_frame", err);
                break;
            }

            // AVFrame → QVideoFrame，发送到 sink
            QVideoFrame qvf = toQVideoFrame(m_frame);
            if (qvf.isValid()) {
                emit frameReady(qvf);
            }

            av_frame_unref(m_frame);
        }
    }

    qCInfo(vpPull) << "Decode loop exited for" << m_url;
    cleanup();
    emit statusChanged(VideoPullService::Idle);
}

QVideoFrame VideoPullWorker::toQVideoFrame(AVFrame* src) {
    if (!src || src->width <= 0 || src->height <= 0)
        return {};

    const int w = src->width;
    const int h = src->height;
    const auto avFmt = static_cast<AVPixelFormat>(src->format);

    QVideoFrameFormat::PixelFormat qtFmt = QVideoFrameFormat::Format_Invalid;
    int planeCount = 0;
    bool needSws = true;

    for (const auto& m : kDirectMap) {
        if (m.avFmt == avFmt) {
            // YUVJ420P 就是 full-range 的 YUV420P，布局完全相同
            qtFmt = m.qtFmt;
            planeCount = m.planes;
            needSws = false;
            break;
        }
    }
    // YUVJ420P：布局与 YUV420P 完全相同，仅色彩范围为 full
    if (avFmt == AV_PIX_FMT_YUVJ420P) {
        qtFmt = QVideoFrameFormat::Format_YUV420P;
        planeCount = 3;
        needSws = false;
    }

    AVFrame* workFrame = src;
    AVFrame* swsFrame = nullptr;

    // 需要通过 SWS 转换为 NV12
    if (needSws) {
        swsFrame = av_frame_alloc();
        if (!swsFrame) return {};
        swsFrame->format = AV_PIX_FMT_NV12;
        swsFrame->width = w;
        swsFrame->height = h;
        if (av_frame_get_buffer(swsFrame, 32) < 0) {
            av_frame_free(&swsFrame);
            return {};
        }

        m_sws = sws_getCachedContext(
            m_sws,
            w, h, avFmt,
            w, h, AV_PIX_FMT_NV12,
            SWS_FAST_BILINEAR,
            nullptr, nullptr, nullptr
        );
        if (!m_sws) {
            av_frame_free(&swsFrame);
            return {};
        }

        sws_scale(m_sws,
            src->data, src->linesize, 0, h,
            swsFrame->data, swsFrame->linesize
        );
        av_frame_copy_props(swsFrame, src);
        workFrame = swsFrame;
        qtFmt = QVideoFrameFormat::Format_NV12;
        planeCount = 2;
    }

    // 构造 QVideoFrameFormat：复制色彩元信息
    QVideoFrameFormat fmt(QSize(w, h), qtFmt);
    fmt.setColorRange(fromAvColorRange(workFrame->color_range));
    fmt.setColorSpace(fromAvColorSpace(workFrame->colorspace));
    fmt.setColorTransfer(fromAvColorTrc(workFrame->color_trc));

    // YUVJ420P 强制 full range
    if (avFmt == AV_PIX_FMT_YUVJ420P)
        fmt.setColorRange(QVideoFrameFormat::ColorRange_Full);

    QVideoFrame qvf(fmt);
    if (!qvf.map(QVideoFrame::WriteOnly)) {
        if (swsFrame) av_frame_free(&swsFrame);
        return {};
    }

    // 逐平面拷贝（支持 stride 不对齐的情况）
    for (int p = 0; p < planeCount && p < 4; ++p) {
        int srcStride = workFrame->linesize[p];
        int dstStride = qvf.bytesPerLine(p);
        uint8_t* dstPtr = qvf.bits(p);
        uint8_t* srcPtr = workFrame->data[p];

        if (!srcPtr || !dstPtr) break;

        // 高度：亮度平面为 h，色度平面为 h/2（或 NV12 的 UV 交织平面）
        int planeH = h;
        if (p > 0) planeH = (qtFmt == QVideoFrameFormat::Format_YUV420P) ? h / 2 : h / 2;

        const int copyWidth = std::min(srcStride, dstStride);
        for (int y = 0; y < planeH; ++y) {
            std::memcpy(dstPtr + y * dstStride, srcPtr + y * srcStride, copyWidth);
        }
    }

    qvf.unmap();
    if (swsFrame) av_frame_free(&swsFrame);
    return qvf;
}

void VideoPullWorker::cleanup() {
    if (m_sws) {
        sws_freeContext(m_sws);
        m_sws = nullptr;
    }
    av_frame_free(&m_frame);
    av_packet_free(&m_pkt);
    avcodec_free_context(&m_dec);
    if (m_ifmt) {
        avformat_close_input(&m_ifmt);
        m_ifmt = nullptr;
    }
    m_vStreamIndex = -1;
}

VideoPullService::VideoPullService(QObject* parent)
    : QObject(parent)
{
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &VideoPullService::doReconnect);
}

VideoPullService::~VideoPullService() {
    stop();
}

void VideoPullService::classBegin() {}

void VideoPullService::componentComplete() {
    play();
}

QString VideoPullService::url() const {
    return m_url;
}

void VideoPullService::setUrl(const QString& url) {
    if (m_url == url) return;
    m_url = url;
    emit urlChanged(m_url);
}

VideoPullService::Status VideoPullService::status() const {
    return m_status;
}

void VideoPullService::setStatus(const Status s) {
    if (m_status == s) return;
    m_status = s;
    emit statusChanged(s);
}

int VideoPullService::reconnectTimes() const {
    return m_reconnectTimes;
}

void VideoPullService::setReconnectTimes(const int times) {
    if (m_reconnectTimes == times) return;
    m_reconnectTimes = times;
    emit reconnectTimesChanged(times);
}

QObject* VideoPullService::videoOutput() const {
    return m_videoOutput;
}

void VideoPullService::setVideoOutput(QObject* output) {
    if (m_videoOutput == output) return;

    // 从传入对象中提取 QVideoSink（兼容 QVideoSink 本身和带 videoSink 属性的 QML VideoOutput）
    QVideoSink* sink = qobject_cast<QVideoSink*>(output);
    if (!sink && output) {
        auto* mo = output->metaObject();
        mo->invokeMethod(output, "videoSink", Q_RETURN_ARG(QVideoSink*, sink));
    }

    m_videoOutput = output;
    emit videoOutputChanged();

    // 断开旧 sink 与 worker 的连接
    if (m_worker && m_videoSink) {
        disconnect(m_worker, &VideoPullWorker::frameReady, m_videoSink, &QVideoSink::setVideoFrame);
    }

    m_videoSink = sink;

    // 连接新 sink（DirectConnection：在 worker 线程直接调用，最低延迟）
    if (m_worker && m_videoSink) {
        reconnectSinkToWorker_for(m_worker);
    }
}

void VideoPullService::play() {
    if (m_url.isEmpty()) {
        qCWarning(vpPull) << "play() called with empty URL, ignored";
        return;
    }

    // 开启无限重连模式，清零计数器
    m_active = true;
    setReconnectTimes(0);

    // 停止可能正在等待的重连定时器
    m_reconnectTimer->stop();

    startWorker();
}

void VideoPullService::stop() {
    // 关闭重连，必须在停止 worker 前设置（防止 finished 信号再次触发重连）
    m_active = false;
    m_reconnectTimer->stop();

    if (!m_worker) {
        setStatus(Idle);
        return;
    }

    m_worker->requestStop();
    if (!m_worker->wait(3000)) {
        qCWarning(vpPull) << "Worker thread did not stop in 3s, terminating";
        m_worker->terminate();
        m_worker->wait(1000);
    }
    // deleteLater 已由 finished 信号关联
    m_worker = nullptr;
    setStatus(Idle);
}

void VideoPullService::onWorkerStatusChanged(int s) {
    const auto status = static_cast<Status>(s);
    // 成功建流后重置重连计数器
    if (status == Playing) {
        setReconnectTimes(0);
    }
    setStatus(status);
}

void VideoPullService::onWorkerError(int code, const QString& msg) {
    emit errorOccurred(code, msg);
}

void VideoPullService::startWorker() {
    // 若已有 worker 在运行，先请求停止（不默工等待，它的 finished 信号会清理指针）
    if (m_worker) {
        m_worker->requestStop();
        // 短暫等待以避免旧 worker 还没退出就创建新的
        if (!m_worker->wait(3000)) {
            m_worker->terminate();
            m_worker->wait(500);
        }
        m_worker = nullptr;
    }

    auto* worker = new VideoPullWorker(this);
    worker->setUrl(m_url);

    connect(worker, &VideoPullWorker::statusChanged, this, &VideoPullService::onWorkerStatusChanged);
    connect(worker, &VideoPullWorker::errorOccurred,  this, &VideoPullService::onWorkerError);
    reconnectSinkToWorker_for(worker);

    // 线程结束：释放内存，并按需调度重连
    connect(worker, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QThread::finished, this, [this, worker] {
        if (m_worker == worker)
            m_worker = nullptr;
        // 如果仍处于活跃状态则调度重连
        scheduleReconnect();
    });

    m_worker = worker;
    m_worker->start();
}

void VideoPullService::reconnectSinkToWorker_for(VideoPullWorker* worker) {
    if (!worker || !m_videoSink) return;
    connect(worker, &VideoPullWorker::frameReady,
        m_videoSink, &QVideoSink::setVideoFrame,
        Qt::DirectConnection);
}

void VideoPullService::scheduleReconnect() {
    if (!m_active) return; // stop() 已调用，不重连

    setReconnectTimes(m_reconnectTimes + 1);

    // 指数退避：1s → 2s → 4s → 8s → 16s → 32s（最大 kMaxReconnectMs）
    const int shift = std::min(m_reconnectTimes - 1, 5);
    const int delay = std::min(kBaseReconnectMs << shift, kMaxReconnectMs);

    qCInfo(vpPull) << "Schedule reconnect #" << m_reconnectTimes
                   << "in" << delay << "ms, url:" << m_url;

    setStatus(Connecting);
    m_reconnectTimer->start(delay);
}

void VideoPullService::doReconnect() {
    if (!m_active) return;
    qCInfo(vpPull) << "Reconnecting (attempt" << m_reconnectTimes << ")...";
    startWorker();
}
