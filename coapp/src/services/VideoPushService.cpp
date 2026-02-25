#include "VideoPushService.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
}

#include <QtCore/QDebug>
#include <QtCore/QLoggingCategory>
#include <QtCore/QMutexLocker>
#include <QtCore/QThread>
#include <QtCore/QTimer>

Q_LOGGING_CATEGORY(vpService, "Services.VideoPushService")

using namespace FFMpegHelper;

namespace {
    /// 选择合适的编码像素格式
    AVPixelFormat chooseEncoderPixFmt(const FFEncoderType encType, const QVideoFrameFormat::PixelFormat srcFmt) {
        const QList<AVPixelFormat> preferred = getPreferredPixFmts(encType);

        // 如果 srcFmt 在 preferred 中，则直接使用该格式
        const AVPixelFormat srcAvFmt = toAvPixelFormat(srcFmt);
        if (preferred.contains(srcAvFmt)) {
            return srcAvFmt;
        }

        // 否则使用 preferred 中的第一个格式
        return preferred.isEmpty() ? AV_PIX_FMT_NONE : preferred.first();
    }

    /// 打印 ffmpeg 错误信息到 qWarning
    void logAvError(const char* prefix, const int err) {
        char buf[AV_ERROR_MAX_STRING_SIZE] = {};
        av_strerror(err, buf, sizeof(buf));
        qCWarning(vpService) << prefix << ":" << buf;
    }

    /// RAII mapper to guarantee unmap
    struct QVideoFrameMapper {
        QVideoFrame& vf;
        bool mapped = false;

        explicit QVideoFrameMapper(QVideoFrame& v) : vf(v) {
            mapped = vf.map(QVideoFrame::ReadOnly);
        }

        ~QVideoFrameMapper() {
            if (mapped) vf.unmap();
        }
    };

    /// 将 QVideoFrame 拷贝转换为 AVFrame
    AVFrame* toAVFrameCopy(QVideoFrame& vf, const int64_t pts) {
        if (!vf.isValid()) return nullptr;

        const QVideoFrameFormat fmt = vf.surfaceFormat();
        const int w = fmt.frameWidth();
        const int h = fmt.frameHeight();
        // 检查偶数宽高
        if ((w & 1) || (h & 1)) {
            return nullptr;
        }

        const AVPixelFormat avFmt = toAvPixelFormat(fmt.pixelFormat());
        if (avFmt == AV_PIX_FMT_NONE) return nullptr;

        QVideoFrameMapper mapper(vf);
        if (!mapper.mapped) return nullptr;

        // Prepare src pointers/stride from QVideoFrame
        uint8_t* srcData[4] = {nullptr};
        int srcLinesize[4] = {};
        const int srcPlaneCount = vf.planeCount();
        for (int p = 0; p < srcPlaneCount && p < 4; ++p) {
            srcData[p] = vf.bits(p);
            srcLinesize[p] = vf.bytesPerLine(p);
        }

        // Allocate destination AVFrame
        AVFrame* dst = av_frame_alloc();
        if (!dst) return nullptr;

        dst->format = avFmt;
        dst->color_range = toAvColorRange(fmt.colorRange());
        dst->colorspace = toAvColorSpace(fmt.colorSpace());
        dst->color_trc = toAvColorTrc(fmt.colorTransfer());
        dst->width = w;
        dst->height = h;
        dst->pts = pts;

        // Allocate dst buffers with alignment
        if (av_frame_get_buffer(dst, 32) < 0) {
            av_frame_free(&dst);
            return nullptr;
        }
        if (av_frame_make_writable(dst) < 0) {
            av_frame_free(&dst);
            return nullptr;
        }

        av_image_copy(dst->data, dst->linesize, srcData, srcLinesize, avFmt, w, h);
        return dst;
    }

    /// 填充黑底
    void fill_black_420(AVFrame* dst) {
        const int w = dst->width, h = dst->height;

        if (dst->format == AV_PIX_FMT_YUV420P) {
            for (int y = 0; y < h; y++)
                std::memset(dst->data[0] + y * dst->linesize[0], 0, w); // Y=0
            for (int y = 0; y < h / 2; y++) {
                std::memset(dst->data[1] + y * dst->linesize[1], 128, w / 2); // U=128
                std::memset(dst->data[2] + y * dst->linesize[2], 128, w / 2); // V=128
            }
            return;
        }
        if (dst->format == AV_PIX_FMT_NV12) {
            for (int y = 0; y < h; y++)
                std::memset(dst->data[0] + y * dst->linesize[0], 0, w); // Y=0
            for (int y = 0; y < h / 2; y++)
                std::memset(dst->data[1] + y * dst->linesize[1], 128, w); // UV=128
            return;
        }
        throw std::runtime_error("fill_black_420: dst must be NV12 or YUV420P");
    }

    /// 把 src 贴到 dst 的 (off_x, off_y) 位置
    void blit_420(const AVFrame* src, AVFrame* dst, const int off_x, const int off_y) {
        const int sw = src->width, sh = src->height;
        const int dw = dst->width, dh = dst->height;

        if (src->format != dst->format) throw std::runtime_error("blit_420: format mismatch");
        if (off_x < 0 || off_y < 0 || off_x + sw > dw || off_y + sh > dh)
            throw std::runtime_error("blit_420: out of bounds");

        for (int y = 0; y < sh; y++) {
            std::memcpy(dst->data[0] + (off_y + y) * dst->linesize[0] + off_x,
                src->data[0] + y * src->linesize[0],
                sw);
        }
        if (dst->format == AV_PIX_FMT_NV12) {
            for (int y = 0; y < sh / 2; y++) {
                std::memcpy(dst->data[1] + (off_y / 2 + y) * dst->linesize[1] + off_x,
                    src->data[1] + y * src->linesize[1],
                    sw);
            }
        }
        else { // YUV420P
            for (int y = 0; y < sh / 2; y++) {
                std::memcpy(dst->data[1] + (off_y / 2 + y) * dst->linesize[1] + (off_x / 2),
                    src->data[1] + y * src->linesize[1],
                    sw / 2);
                std::memcpy(dst->data[2] + (off_y / 2 + y) * dst->linesize[2] + (off_x / 2),
                    src->data[2] + y * src->linesize[2],
                    sw / 2);
            }
        }
    }
} // namespace

void PushConfig::setResolution(const QSize& sz) {
    resWidth = sz.width() & ~1;
    resHeight = sz.height() & ~1;
}

FFColorFormat::FFColorFormat(const QVideoFrameFormat& fmt, const AVColorPrimaries prim) {
    colorRange = toAvColorRange(fmt.colorRange() ? fmt.colorRange() : QVideoFrameFormat::ColorRange_Video);
    colorSpace = toAvColorSpace(fmt.colorSpace() ? fmt.colorSpace() : QVideoFrameFormat::ColorSpace_BT709);
    colorTrc = toAvColorTrc(fmt.colorTransfer() ? fmt.colorTransfer() : QVideoFrameFormat::ColorTransfer_BT709);
    colorPrim = prim;
}

void LetterboxScaler420::reset() {
    if (m_sws) {
        sws_freeContext(m_sws);
        m_sws = nullptr;
    }
}

AVFrame* LetterboxScaler420::convert(AVFrame** src_io, const int dst_w, const int dst_h, const AVPixelFormat dst_fmt,
                                     const int sws_flags) {
    if (!src_io || !*src_io) return nullptr;
    AVFrame* src = *src_io;

    if (src->width <= 0 || src->height <= 0) return nullptr;
    if (dst_w <= 0 || dst_h <= 0) return nullptr;

    if (!(dst_fmt == AV_PIX_FMT_YUV420P || dst_fmt == AV_PIX_FMT_NV12))
        return nullptr;

    const int src_w = src->width;
    const int src_h = src->height;
    const auto src_fmt = static_cast<AVPixelFormat>(src->format);

    // 快速路径 A：同格式同尺寸 => move_ref 零拷贝
    if (src_fmt == dst_fmt && src_w == dst_w && src_h == dst_h) {
        AVFrame* out = av_frame_alloc();
        if (!out) return nullptr;
        av_frame_move_ref(out, src);
        av_frame_free(src_io);
        return out;
    }

    int scaled_w = 0, scaled_h = 0, off_x = 0, off_y = 0;

    if (src_w == dst_w && src_h == dst_h) {
        // 源==目标: 不缩放, 铺满
        scaled_w = dst_w;
        scaled_h = dst_h;
        off_x = 0;
        off_y = 0;
    }
    else if (src_w < dst_w && src_h < dst_h) {
        // 源完全小于目标: 不缩放, 居中贴
        scaled_w = src_w;
        scaled_h = src_h;
        off_x = (dst_w - scaled_w) / 2;
        off_y = (dst_h - scaled_h) / 2;
    }
    else {
        // 其他情况: 等比缩放到画布内最大
        const double s = std::min((double)dst_w / src_w, (double)dst_h / src_h);
        scaled_w = (int)std::lround(src_w * s);
        scaled_h = (int)std::lround(src_h * s);
        off_x = (dst_w - scaled_w) / 2;
        off_y = (dst_h - scaled_h) / 2;
    }

    // 4:2:0 对齐
    scaled_w &= ~1;
    scaled_h &= ~1;
    off_x &= ~1;
    off_y &= ~1;

    // 分配 dst
    AVFrame* dst = av_frame_alloc();
    if (!dst) return nullptr;
    dst->format = dst_fmt;
    dst->width = dst_w;
    dst->height = dst_h;

    if (av_frame_get_buffer(dst, 32) < 0 || av_frame_make_writable(dst) < 0) {
        av_frame_free(&dst);
        return nullptr;
    }

    // 黑底
    fill_black_420(dst);

    // 快速路径 B：不缩放 + 同格式 => memcpy 贴图
    if (src_fmt == dst_fmt && scaled_w == src_w && scaled_h == src_h) {
        blit_420(src, dst, off_x, off_y);
        av_frame_copy_props(dst, src);
        av_frame_free(src_io);
        return dst;
    }

    // 复用/更新 SWS context
    m_sws = sws_getCachedContext(
        m_sws,
        src_w, src_h, src_fmt,
        scaled_w, scaled_h, dst_fmt,
        sws_flags,
        nullptr, nullptr, nullptr
    );
    if (!m_sws) {
        av_frame_free(&dst);
        throw std::runtime_error("sws_getCachedContext failed");
    }

    // 写入子区域
    uint8_t* out_data[4] = {nullptr, nullptr, nullptr, nullptr};
    int out_linesize[4] = {0, 0, 0, 0};

    out_data[0] = dst->data[0] + off_y * dst->linesize[0] + off_x;
    out_linesize[0] = dst->linesize[0];

    if (dst_fmt == AV_PIX_FMT_NV12) {
        out_data[1] = dst->data[1] + (off_y / 2) * dst->linesize[1] + off_x;
        out_linesize[1] = dst->linesize[1];
    }
    else { // YUV420P/YUVJ420P
        out_data[1] = dst->data[1] + (off_y / 2) * dst->linesize[1] + (off_x / 2);
        out_linesize[1] = dst->linesize[1];
        out_data[2] = dst->data[2] + (off_y / 2) * dst->linesize[2] + (off_x / 2);
        out_linesize[2] = dst->linesize[2];
    }

    sws_scale(m_sws, src->data, src->linesize, 0, src_h, out_data, out_linesize);

    av_frame_copy_props(dst, src);
    av_frame_free(src_io);
    return dst;
}

AVFrame* LetterboxScaler420::operator()(AVFrame** src_io, const int dst_w, const int dst_h, const AVPixelFormat dst_fmt,
                                        const int sws_flags) {
    return convert(src_io, dst_w, dst_h, dst_fmt, sws_flags);
}

VideoPushWorker::VideoPushWorker(QObject* parent)
    : QObject(parent) {
    // 定时上报统计（每秒一次）
    m_statsReportTimer = new QTimer(this);
    m_statsReportTimer->setInterval(1000);
    connect(m_statsReportTimer, &QTimer::timeout, this, [this] {
        // 计算近 1 秒帧率和码率（两者共用同一计时窗口）
        const qint64 elapsed = m_fpsTimer.restart();
        {
            QMutexLocker lk(&m_mutex);
            m_stats.currentFps =
                elapsed > 0 ? static_cast<double>(m_fpsCounter) * 1000.0 / static_cast<double>(elapsed) : 0.0;
            m_stats.currentBitrateKbps =
                elapsed > 0 ? static_cast<double>(m_bytesInWindow) * 8.0 / static_cast<double>(elapsed) : 0.0;
        }
        m_fpsCounter = 0;
        m_bytesInWindow = 0;
        emit statsUpdated(m_stats);
    });

    m_fpsTimer.start();
}

VideoPushWorker::~VideoPushWorker() {
    cleanupFFMPEG();
}

PushWorkerState VideoPushWorker::state() const {
    QMutexLocker lk(&m_mutex);
    return m_state;
}

PushStats VideoPushWorker::stats() const {
    QMutexLocker lk(&m_mutex);
    return m_stats;
}

void VideoPushWorker::start(const PushConfig& config) {
    if (state() == PushWorkerState::Error) {
        reset();
    }
    if (state() != PushWorkerState::Idle) {
        qCWarning(vpService) << "start() ignored: state is not Idle";
        return;
    }

    setPushConfig(config);
    m_frameIndex = 0;
    m_fpsCounter = 0;
    m_bytesInWindow = 0;
    m_fpsTimer.restart();
    m_lastEncodeTimestampUs = -1;
    m_frameRateTimer.start();

    // 验证编码器是否存在（快速失败，不分配上下文）
    const char* codecName = getCodecName(config.encoderType);
    if (!codecName || !avcodec_find_encoder_by_name(codecName)) {
        emit errorOccurred(-1, QString("Encoder not found in this FFMPEG build: ") % codecName);
        setState(PushWorkerState::Error);
        return;
    }

    // 完整初始化（openEncoder + openRtspOutput）需要真实宽高，
    // 延迟到首帧 pushFrame() 到来时执行；此处仅切换到 Starting 状态。
    setState(PushWorkerState::Starting);
}

void VideoPushWorker::stop() {
    const auto s = state();
    if (s != PushWorkerState::Streaming && s != PushWorkerState::Starting) {
        qCWarning(vpService) << "stop() ignored: not in Streaming/Starting state";
        return;
    }

    setState(PushWorkerState::Stopping);
    m_statsReportTimer->stop();

    // Flush 编码器（仅在已完成初始化时执行）
    if (m_enc && m_ofmt) {
        encodeAndWrite(nullptr);
        av_write_trailer(m_ofmt);
    }

    cleanupFFMPEG();
    setState(PushWorkerState::Idle);
}

void VideoPushWorker::reset() {
    if (state() != PushWorkerState::Error) {
        qCWarning(vpService()) << "reset() ignored: not in Error state";
        return;
    }
    m_statsReportTimer->stop();
    cleanupFFMPEG();
    setState(PushWorkerState::Idle);
}

void VideoPushWorker::pushFrame(const QVideoFrame& frame) {
    // Starting：首帧到达，执行完整的延迟初始化
    if (const auto s = state(); s == PushWorkerState::Starting) {
        if (!frame.isValid()) return;

        // 偶数宽高
        const int w = frame.width(), h = frame.height();
        if ((w & 1) || (h & 1)) {
            qCWarning(vpService) << "First frame has odd dimensions, skip init";
            return;
        }

        // 确定像素和颜色格式
        m_encPixFmt = chooseEncoderPixFmt(m_config.encoderType, frame.pixelFormat());
        m_encColorFmt = FFColorFormat{frame.surfaceFormat()};

        // 开始执行编码
        if (!openEncoder() || !openRtspOutput()) {
            cleanupFFMPEG();
            setState(PushWorkerState::Error);
            return;
        }

        m_statsReportTimer->start();
        setState(PushWorkerState::Streaming);
    }
    else if (s != PushWorkerState::Streaming) {
        QMutexLocker lk(&m_mutex);
        ++m_stats.framesDropped;
        return;
    }

    if (!frame.isValid()) {
        QMutexLocker lk(&m_mutex);
        ++m_stats.framesDropped;
        return;
    }

    // 丢帧策略：源帧率 > 目标帧率时，按目标帧间隔丢弃多余帧
    if (frame.streamFrameRate() > m_config.fps) {
        const qint64 nowUs = m_frameRateTimer.nsecsElapsed() / 1000;
        const qint64 intervalUs = 1'000'000LL / m_config.fps;
        if (m_lastEncodeTimestampUs >= 0 && (nowUs - m_lastEncodeTimestampUs) < intervalUs) {
            QMutexLocker lk(&m_mutex);
            ++m_stats.framesDropped;
            return;
        }
        m_lastEncodeTimestampUs = nowUs;
    }

    auto vf = const_cast<QVideoFrame&>(frame);
    // 步骤 1：转换为AVFrame
    AVFrame* avf = toAVFrameCopy(vf, m_frameIndex++);
    if (!avf) {
        QMutexLocker lk(&m_mutex);
        ++m_stats.framesDropped;
        return;
    }

    // 步骤 2：格式转换,缩放,黑底填充
    AVFrame* f = scaler(&avf, m_config.resWidth, m_config.resHeight, m_encPixFmt);
    if (!f) {
        av_frame_free(&avf); // 释放源帧
        QMutexLocker lk(&m_mutex);
        ++m_stats.framesDropped;
        return;
    }

    // 步骤 3: 编码写流
    const bool ok = encodeAndWrite(f);
    av_frame_free(&f);

    QMutexLocker lk(&m_mutex);
    if (ok) {
        ++m_stats.framesEncoded;
        ++m_fpsCounter;
    }
    else {
        ++m_stats.framesDropped;
    }
}

void VideoPushWorker::setPushConfig(const PushConfig& config) {
    m_config = config;
}

bool VideoPushWorker::openEncoder() {
    // 获取编码器
    const char* codecName = getCodecName(m_config.encoderType);
    const AVCodec* codec = avcodec_find_encoder_by_name(codecName);
    if (!codec) {
        emit errorOccurred(-1, QString("Encoder not found: ") % codecName);
        return false;
    }

    // 创建编码器上下文
    m_enc = avcodec_alloc_context3(codec);
    if (!m_enc) {
        emit errorOccurred(-1, "avcodec_alloc_context3 failed");
        return false;
    }

    // 填充编码参数
    m_enc->codec_type = AVMEDIA_TYPE_VIDEO;
    m_enc->width = m_config.resWidth;
    m_enc->height = m_config.resHeight;
    m_enc->pix_fmt = m_encPixFmt;
    m_enc->time_base = {1, m_config.fps};
    m_enc->framerate = {m_config.fps, 1};
    m_enc->color_range = m_encColorFmt.colorRange;
    m_enc->colorspace = m_encColorFmt.colorSpace;
    m_enc->color_trc = m_encColorFmt.colorTrc;
    m_enc->color_primaries = m_encColorFmt.colorPrim;

    // 从配置中提取其他编码参数
    std::visit([&](auto&& opts) {
        using T = std::decay_t<decltype(opts)>;
        const CommonEncodeOptions& common = opts;

        m_enc->bit_rate = common.bitrate;
        m_enc->max_b_frames = common.maxBFrames;
        if (common.gopSize > 0) {
            m_enc->gop_size = common.gopSize;
        }
        m_enc->profile = common.profile;

        if constexpr (std::is_same_v<T, X264EncodeOptions>) {
            av_opt_set(m_enc->priv_data, "preset", opts.preset.toUtf8().constData(), 0);
            av_opt_set(m_enc->priv_data, "tune", opts.tune.toUtf8().constData(), 0);
        }
        else if constexpr (std::is_same_v<T, VTEncodeOptions>) {
            av_opt_set_int(m_enc->priv_data, "realtime", opts.realtime ? 1 : 0, 0);
            av_opt_set_int(m_enc->priv_data, "allow_sw", opts.allowSWFallback ? 1 : 0, 0);
            av_opt_set_int(m_enc->priv_data, "prio_speed", opts.prioritizeSpeed ? 1 : 0, 0);
            av_opt_set_int(m_enc->priv_data, "spatial_aq", opts.spatialAQ ? 1 : 0, 0);
        }
        else if constexpr (std::is_same_v<T, QSVEncodeOptions>) {
            av_opt_set(m_enc->priv_data, "preset", opts.preset.toUtf8().constData(), 0);
            av_opt_set_int(m_enc->priv_data, "async_depth", opts.asyncDepth, 0);
        }
        else if constexpr (std::is_same_v<T, NvEncEncodeOptions>) {
            av_opt_set(m_enc->priv_data, "preset", opts.preset.toUtf8().constData(), 0);
            av_opt_set(m_enc->priv_data, "tune", opts.tune.toUtf8().constData(), 0);
            av_opt_set_int(m_enc->priv_data, "lookahead", opts.lookahead, 0);
        }
    }, m_config.encoderOptions);

    // RTSP 推流必须在打开编码器前设置 GLOBAL_HEADER，
    // 使 SPS/PPS 写入 extradata 而非内联在每帧中。
    // 对 libx264 无害；对 h264_videotoolbox 等硬件编码器是必需的，
    // 否则拉流端因拿不到解码参数而报 "non-existing PPS referenced"。
    m_enc->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // 打开编码器
    const int err = avcodec_open2(m_enc, codec, nullptr);
    if (err < 0) {
        logAvError("avcodec_open2", err);
        emit errorOccurred(err, QString("Failed to open encoder: %1") % codecName);
        return false;
    }

    return true;
}

bool VideoPushWorker::openRtspOutput() {
    if (!m_enc) return false;

    // 1) 创建 RTSP 输出上下文
    int err = avformat_alloc_output_context2(
        &m_ofmt, nullptr, "rtsp", m_config.rtspUrl.toUtf8().constData());
    if (err < 0 || !m_ofmt) {
        logAvError("avformat_alloc_output_context2", err);
        emit errorOccurred(err, "Failed to create RTSP output context");
        return false;
    }

    // 2) 新建视频流
    m_vst = avformat_new_stream(m_ofmt, nullptr);
    if (!m_vst) {
        emit errorOccurred(-1, "avformat_new_stream failed");
        return false;
    }

    // 3) 复制编码参数到流
    m_vst->time_base = m_enc->time_base;
    err = avcodec_parameters_from_context(m_vst->codecpar, m_enc);
    if (err < 0) {
        logAvError("avcodec_parameters_from_context", err);
        emit errorOccurred(err, "avcodec_parameters_from_context failed");
        return false;
    }

    // 4) 配置 RTSP 相关输出选项
    AVDictionary* opts = nullptr;
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
    av_dict_set_int(&opts, "muxdelay", 0, 0);
    av_dict_set_int(&opts, "muxpreload", 0, 0);
    av_dict_set_int(&opts, "stimeout", m_config.connectTimeoutUs, 0);

    // 5) 打开IO, 写入头部（RTSP ANNOUNCE + SETUP + RECORD）
    err = avformat_write_header(m_ofmt, &opts);
    av_dict_free(&opts);
    if (err < 0) {
        logAvError("avformat_write_header", err);
        emit errorOccurred(err, "Failed to connect to RTSP server");
        return false;
    }

    // 分配复用 Packet
    m_pkt = av_packet_alloc();
    if (!m_pkt) {
        emit errorOccurred(-1, "av_packet_alloc failed");
        return false;
    }

    return true;
}

void VideoPushWorker::cleanupFFMPEG() {
    av_packet_free(&m_pkt);
    avcodec_free_context(&m_enc);

    if (m_ofmt) {
        if (m_ofmt->pb && !(m_ofmt->oformat->flags & AVFMT_NOFILE))
            avio_closep(&m_ofmt->pb);
        avformat_free_context(m_ofmt);
    }

    m_ofmt = nullptr;
    m_vst = nullptr;
    m_enc = nullptr;
    m_pkt = nullptr;

    scaler.reset();

    m_frameIndex = 0;
}

bool VideoPushWorker::encodeAndWrite(AVFrame* frame) {
    if (!m_enc || !m_ofmt || !m_vst || !m_pkt) return false;

    int err = avcodec_send_frame(m_enc, frame);
    if (err < 0 && err != AVERROR_EOF) {
        logAvError("avcodec_send_frame", err);
        return false;
    }

    while (true) {
        err = avcodec_receive_packet(m_enc, m_pkt);
        if (err == AVERROR(EAGAIN) || err == AVERROR_EOF) break;
        if (err < 0) {
            logAvError("avcodec_receive_packet", err);
            return false;
        }

        // 转换 time_base
        av_packet_rescale_ts(m_pkt, m_enc->time_base, m_vst->time_base);
        m_pkt->stream_index = m_vst->index;

        // 写包
        const auto pktSize = static_cast<quint64>(m_pkt->size);
        err = av_interleaved_write_frame(m_ofmt, m_pkt);
        av_packet_unref(m_pkt);

        if (err < 0) {
            logAvError("av_interleaved_write_frame", err);
            // 网络写出失败视为致命错误，转入 Error 状态
            m_statsReportTimer->stop();
            cleanupFFMPEG();
            emit errorOccurred(err, "RTSP write failed (connection lost?)");
            setState(PushWorkerState::Error);
            return false;
        }

        m_bytesInWindow += pktSize;
    }
    return true;
}

void VideoPushWorker::setState(PushWorkerState s) {
    {
        QMutexLocker lk(&m_mutex);
        m_state = s;
    }
    emit stateChanged(s);
}

VideoPushService::VideoPushService(QObject* parent)
    : QObject(parent)
{}

VideoPushService::~VideoPushService() {
    if (m_thread && m_thread->isRunning()) {
        // 尝试优雅停止
        if (m_worker && m_worker->state() == PushWorkerState::Streaming) {
            QMetaObject::invokeMethod(m_worker, &VideoPushWorker::stop,
                Qt::BlockingQueuedConnection);
        }
        m_thread->quit();
        m_thread->wait(3000);
    }
}

PushWorkerState VideoPushService::state() const {
    if (!m_worker) return PushWorkerState::Idle;
    return m_worker->state();
}

PushStats VideoPushService::stats() const {
    QMutexLocker lk(&m_statsMutex);
    return m_cachedStats;
}

void VideoPushService::ensureWorkerCreated() {
    if (m_worker) return;

    m_thread = new QThread(this);
    m_worker = new VideoPushWorker; // 无 parent，便于 moveToThread
    m_worker->moveToThread(m_thread);

    // Worker 销毁时清理 thread
    connect(m_thread, &QThread::finished, m_worker, &QObject::deleteLater);

    // 中继信号
    connect(m_worker, &VideoPushWorker::stateChanged,
        this, &VideoPushService::stateChanged);
    connect(m_worker, &VideoPushWorker::errorOccurred,
        this, &VideoPushService::errorOccurred);
    connect(m_worker, &VideoPushWorker::statsUpdated, this,
        [this](const PushStats& st) {
            {
                QMutexLocker lk(&m_statsMutex);
                m_cachedStats = st;
            }
            emit statsUpdated(st);
        });

    m_thread->start();
}

void VideoPushService::start(const PushConfig& config) {
    ensureWorkerCreated();
    QMetaObject::invokeMethod(m_worker,
        [worker = m_worker, config] { worker->start(config); },
        Qt::QueuedConnection);
}

void VideoPushService::stop() {
    if (!m_worker) return;
    QMetaObject::invokeMethod(m_worker, &VideoPushWorker::stop,
        Qt::QueuedConnection);
}

void VideoPushService::reset() {
    if (!m_worker) return;
    QMetaObject::invokeMethod(m_worker, &VideoPushWorker::reset,
        Qt::QueuedConnection);
}

void VideoPushService::pushFrame(const QVideoFrame& frame) {
    if (!m_worker) return;
    QMetaObject::invokeMethod(m_worker,
        [worker = m_worker, frame] { worker->pushFrame(frame); },
        Qt::QueuedConnection);
}
