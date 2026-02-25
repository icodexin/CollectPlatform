#ifndef VIDEOPUSHSERVICE_H
#define VIDEOPUSHSERVICE_H


extern "C" {
#include <libavutil/pixfmt.h>
#include <libavutil/rational.h>
#include <libswscale/swscale.h>
}

#include <QElapsedTimer>
#include <QHash>
#include <QList>
#include <QMutex>
#include <QObject>
#include <QVideoFrame>
#include <QVideoFrameFormat>

#include <functional>
#include <variant>

#include "services/FFmpegHelper.h"

struct AVFormatContext;
struct AVStream;
struct AVCodecContext;
struct AVCodec;
struct AVPacket;
struct AVFrame;
struct SwsContext;

class QTimer;
class QThread;

/// 所有编码器共用的基础编码参数
struct CommonEncodeOptions {
    int64_t bitrate = 20'000'000;           ///< 目标码率（bits/s），CBR 模式
    int gopSize = 0;                        ///< 关键帧间隔, 单位: 帧, 0表示自动
    int maxBFrames = 0;                     ///< 最大 B 帧数，RTSP 低延迟推流推荐设为 0
    int profile = AV_PROFILE_H264_BASELINE; ///< H.264 编码档次
};

/// libx264 软件编码专有选项
struct X264EncodeOptions : CommonEncodeOptions {
    QString preset = "ultrafast"; ///< 编码速度 / 质量权衡，低延迟推流推荐 ultrafast
    QString tune = "zerolatency"; ///< 编码调优，RTSP 推流推荐 zerolatency
};

/// libOpenH264 软件编码专有选项
struct OpenH264EncodeOptions : CommonEncodeOptions {
    // OpenH264 暂无特有选项，保留结构体以便未来扩展
};

/// Apple VideoToolbox 硬件编码专有选项
struct VTEncodeOptions : CommonEncodeOptions {
    bool realtime = true;         ///< 启用实时编码模式，有效降低编码延迟
    bool allowSWFallback = false; ///< 硬件不可用时是否静默回退到软件编码
    bool prioritizeSpeed = true;  ///< 速度优先（prio_speed），牺牲少量质量换延迟
    bool spatialAQ = false;       ///< 空间自适应量化，推流场景建议关闭
};

/// Intel QSV 硬件编码专有选项
struct QSVEncodeOptions : CommonEncodeOptions {
    QString preset = "faster"; ///< 编码 preset：veryfast / faster / fast / medium
    int asyncDepth = 1;        ///< 异步深度，1 = 最低延迟
};

/// NVIDIA NVENC 硬件编码专有选项
struct NvEncEncodeOptions : CommonEncodeOptions {
    QString preset = "p1"; ///< p1（最快）~ p7（最好质量），低延迟推荐 p1
    QString tune = "ull";  ///< ull = 超低延迟，ll = 低延迟，hq = 高质量
    int lookahead = 0;     ///< 前向预测帧数，0 = 关闭（低延迟推荐）
};

/// 多编码器选项的类型安全联合体
using EncoderOptions = std::variant<
    X264EncodeOptions,
    OpenH264EncodeOptions,
    VTEncodeOptions,
    QSVEncodeOptions,
    NvEncEncodeOptions
>;

/// 推流配置
struct PushConfig {
    QString rtspUrl;
    int fps = 30;
    FFEncoderType encoderType = FF_ET_X264;
    EncoderOptions encoderOptions = OpenH264EncodeOptions{};
    int resWidth = 1280;
    int resHeight = 720;
    int connectTimeoutUs = 3'000'000; // 连接 / IO 超时（微秒），0 = 不超时

    void setResolution(const QSize &sz);
};

/// 色彩格式
struct FFColorFormat {
    AVColorRange colorRange = AVCOL_RANGE_MPEG;
    AVColorSpace colorSpace = AVCOL_SPC_BT709;
    AVColorTransferCharacteristic colorTrc = AVCOL_TRC_BT709;
    AVColorPrimaries colorPrim = AVCOL_PRI_BT709;

    FFColorFormat() = default;
    explicit FFColorFormat(const QVideoFrameFormat& fmt, AVColorPrimaries prim = AVCOL_PRI_BT709);
};

/// 推流过程中的实时统计数据，由 Worker 定期通过 statsUpdated() 信号上报
struct PushStats {
    quint64 framesEncoded = 0;       ///< 已成功编码并写出的总帧数
    quint64 framesDropped = 0;       ///< 因格式不支持、编码失败或帧率限制等原因丢弃的帧数
    double currentFps = 0.0;         ///< 近 1 秒内的实际编码帧率（帧/秒）
    double currentBitrateKbps = 0.0; ///< 近 1 秒内的实际推流码率（kbps）
};

/// 推流Worker状态
enum class PushWorkerState {
    Idle,      ///< 空闲，未推流
    Starting,  ///< 正在初始化编码器 / 连接 RTSP
    Streaming, ///< 已连接，正在编码推流
    Stopping,  ///< 正在 flush 缓冲 / 关闭连接
    Error,     ///< 发生不可恢复错误，需调用 reset() 后才能再次 start()
};

/// 针对YUV420P/NV12的黑底填充分辨率缩放转换器
class LetterboxScaler420 {
public:
    LetterboxScaler420() = default;
    ~LetterboxScaler420() { reset(); }

    void reset();

    AVFrame* convert(AVFrame** src_io, int dst_w, int dst_h, AVPixelFormat dst_fmt, int sws_flags = SWS_BILINEAR);

    AVFrame* operator()(AVFrame** src_io, int dst_w, int dst_h, AVPixelFormat dst_fmt, int sws_flags = SWS_BILINEAR);

private:
    SwsContext* m_sws = nullptr;
};

/// 后台视频编码推流 Worker
class VideoPushWorker final : public QObject {
    Q_OBJECT

public:
    explicit VideoPushWorker(QObject* parent = nullptr);
    ~VideoPushWorker() override;

    /// 返回当前 Worker 状态（线程安全）
    PushWorkerState state() const;

    /// 返回最新统计快照（线程安全）
    PushStats stats() const;

public slots:
    /// 使用指定配置启动推流
    void start(const PushConfig& config);

    /// 优雅停止推流
    void stop();

    /// 从 Error 状态复位至 Idle，以便重新调用 start()
    void reset();

    /// 推入一帧
    void pushFrame(const QVideoFrame& frame);

signals:
    void stateChanged(PushWorkerState newState);
    void errorOccurred(int code, const QString& msg);
    void statsUpdated(const PushStats& st);

private:
    /// 设置推流配置
    void setPushConfig(const PushConfig& config);

    /// 创建并打开编码器上下文
    bool openEncoder();

    /// 创建并打开 RTSP 输出容器、视频流
    bool openRtspOutput();

    /// 释放所有 ffmpeg 资源（编码器、格式上下文、包等）
    void cleanupFFMPEG();

    /// 将 AVFrame 送入编码器并将输出 AVPacket 写入 RTSP 流
    bool encodeAndWrite(AVFrame* frame);

    /// 设置当前状态
    void setState(PushWorkerState s);

private:
    // ffmpeg 上下文
    AVFormatContext* m_ofmt = nullptr; ///< RTSP 输出容器上下文
    AVStream* m_vst = nullptr;         ///< 输出视频流（弱引用，由 m_ofmt 管理）
    AVCodecContext* m_enc = nullptr;   ///< 编码器上下文
    AVPacket* m_pkt = nullptr;         ///< 复用 AVPacket，避免每帧分配

    // 视频流参数
    PushConfig m_config;                         ///< 当前推流配置
    AVPixelFormat m_encPixFmt = AV_PIX_FMT_NV12; ///< 编码器像素格式
    FFColorFormat m_encColorFmt;                 ///< 编码器色彩格式

    // 格式转换
    LetterboxScaler420 scaler;  /// 实现像素格式转换+分辨率缩放+黑色背景填充

    // 动态参数
    int64_t m_frameIndex = 0; ///< 帧序号，用作 pts（按 time_base 递增）

    // 状态与统计
    PushWorkerState m_state = PushWorkerState::Idle;
    PushStats m_stats;
    mutable QMutex m_mutex; ///< 保护 m_state / m_stats 的跨线程读取

    // 计数器/计时器
    QElapsedTimer m_fpsTimer;                ///< 帧率与码率统计共用计时器（每秒由 m_statsReportTimer 触发 restart）
    quint64 m_fpsCounter = 0;                ///< 当前计时周期内已编码帧数
    quint64 m_bytesInWindow = 0;             ///< 当前计时周期内已写出字节数（用于码率计算）
    QTimer* m_statsReportTimer = nullptr;    ///< 定时触发 statsUpdated 信号（1 秒）
    QElapsedTimer m_frameRateTimer;          ///< 帧率控制计时器（丢帧策略，与 m_fpsTimer 独立）
    qint64 m_lastEncodeTimestampUs = -1;     ///< 上次编码帧的时间戳（μs），-1 表示尚未编码任何帧
};

/// 视频推流服务
class VideoPushService final : public QObject {
    Q_OBJECT

public:
    explicit VideoPushService(QObject* parent = nullptr);
    ~VideoPushService() override;

    /// 返回当前推流状态（线程安全）
    PushWorkerState state() const;

    /// 返回最新统计快照（线程安全，来自上次 statsUpdated 信号缓存）
    PushStats stats() const;

public slots:
    /// 启动推流
    void start(const PushConfig& config);

    /// 停止推流
    void stop();

    /// 从 Error 状态复位
    void reset();

    /// QVideoFrame 转发给后台 Worker 编码推流
    void pushFrame(const QVideoFrame& frame);

signals:
    void stateChanged(PushWorkerState newState);
    void errorOccurred(int code, const QString& msg);
    void statsUpdated(const PushStats& st);

private:
    void ensureWorkerCreated();

    VideoPushWorker* m_worker = nullptr;
    QThread* m_thread = nullptr;

    PushStats m_cachedStats;     ///< 来自最近一次 statsUpdated 的统计缓存
    mutable QMutex m_statsMutex; ///< 保护 m_cachedStats 的跨线程访问
};

#endif // VIDEOPUSHSERVICE_H
