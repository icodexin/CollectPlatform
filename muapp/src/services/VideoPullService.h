#ifndef VIDEOPULLSERVICE_H
#define VIDEOPULLSERVICE_H

#include <QtCore/QAtomicInt>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtMultimedia/QVideoFrame>
#include <QtMultimedia/QVideoSink>
#include <QtQml/QQmlParserStatus>
#include <QtQml/qqmlregistration.h>

class QTimer;

struct AVFormatContext;
struct AVCodecContext;
struct AVPacket;
struct AVFrame;
struct SwsContext;

class VideoPullWorker final : public QThread {
    Q_OBJECT

public:
    explicit VideoPullWorker(QObject* parent = nullptr);
    ~VideoPullWorker() override;

    void setUrl(const QString& url);

    /// 请求停止（线程安全）
    void requestStop();

signals:
    /// 每解码一帧向外发送一个 QVideoFrame
    void frameReady(const QVideoFrame& frame);

    /// Worker 进入了某个可观测状态（Connecting / Playing / Stopping / Error / Idle）
    void statusChanged(int status); ///< 对应 VideoPullService::Status 枚举值

    /// 发生错误
    void errorOccurred(int code, const QString& msg);

protected:
    void run() override;

private:
    // FFmpeg 中断回调：m_stopRequested 为 1 时返回非零，让阻塞调用立即返回
    static int interruptCallback(void* ctx);

    // AVFrame → QVideoFrame（含像素格式转换）
    QVideoFrame toQVideoFrame(AVFrame* frame);

    // 释放所有 FFmpeg 资源
    void cleanup();

private:
    // 配置
    QString m_url;

    // 停止标志（原子操作，run() 循环中读取）
    QAtomicInt m_stopRequested{0};

    // FFmpeg 上下文
    AVFormatContext* m_ifmt = nullptr; ///< 输入格式上下文
    AVCodecContext* m_dec = nullptr;   ///< 解码器上下文
    AVPacket* m_pkt = nullptr;         ///< 复用 packet，避免每帧分配
    AVFrame* m_frame = nullptr;        ///< 复用 frame
    SwsContext* m_sws = nullptr;       ///< 像素格式/尺寸转换（按需使用）
    int m_vStreamIndex = -1;           ///< 视频流索引
};

class VideoPullService : public QObject, public QQmlParserStatus {
    Q_OBJECT
    QML_ELEMENT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(QString url READ url WRITE setUrl NOTIFY urlChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QObject* videoOutput READ videoOutput WRITE setVideoOutput NOTIFY videoOutputChanged)
    Q_PROPERTY(int reconnectTimes READ reconnectTimes NOTIFY reconnectTimesChanged)

public:
    enum Status {
        Idle       = 0, ///< 空闲，未主动拉流
        Connecting = 1, ///< 正在连接 / 协商流信息（含重连等待期）
        Playing    = 2, ///< 正在解码输出帧
        Error      = 3, ///< 发生不可恢复错误（stop() 后进入）
    };
    Q_ENUM(Status)

    explicit VideoPullService(QObject* parent = nullptr);
    ~VideoPullService() override;

    void classBegin() override;
    void componentComplete() override;

    QString url() const;
    void setUrl(const QString& url);

    Status status() const;

    QObject* videoOutput() const;
    void setVideoOutput(QObject* output);

    /// 自 play() 以来的累计重连尝试次数；成功建流后重置为 0
    int reconnectTimes() const;

    /// 开始拉流，并启用无限自动重连
    Q_INVOKABLE void play();

    /// 停止拉流，同时关闭自动重连
    Q_INVOKABLE void stop();

signals:
    void urlChanged(const QString& url);
    void statusChanged(Status status);
    void videoOutputChanged();
    void errorOccurred(int code, const QString& msg);
    void reconnectTimesChanged(int times);

private:
    void setStatus(Status s);
    void setReconnectTimes(int times);
    void onWorkerStatusChanged(int s);
    void onWorkerError(int code, const QString& msg);

    /// 提取自 play()，真正创建并启动 Worker
    void startWorker();

    /// 把指定 worker 的 frameReady 连接到当前 m_videoSink
    void reconnectSinkToWorker_for(VideoPullWorker* worker);

    /// Worker 退出后（出错/EOF）按指数退避调度重连
    void scheduleReconnect();

    /// 重连定时器超时槽：实际执行重连
    void doReconnect();

    // 基本状态
    QString m_url;
    Status  m_status = Idle;
    bool    m_componentReady = false;

    // Worker
    VideoPullWorker* m_worker      = nullptr;
    QObject*         m_videoOutput = nullptr; ///< 用户传入的输出对象（VideoOutput / QVideoSink）
    QVideoSink*      m_videoSink   = nullptr; ///< 从 m_videoOutput 中提取的 sink

    // 重连
    bool    m_active         = false;    ///< play() 后为 true，stop() 后为 false
    int     m_reconnectTimes = 0;        ///< 累计重连次数（成功后清零）
    QTimer* m_reconnectTimer = nullptr;  ///< 重连延迟定时器

    static constexpr int kBaseReconnectMs = 1000;  ///< 初始重连间隔 1 s
    static constexpr int kMaxReconnectMs  = 32000; ///< 最大重连间隔 32 s
};

#endif // VIDEOPULLSERVICE_H
