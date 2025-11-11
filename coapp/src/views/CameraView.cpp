#include "CameraView.h"

VideoWidget::VideoWidget(QWidget *parent):QWidget(parent) {
    setMinimumSize(200,300);

    init();
    videofromcamera = new VideoFromCamera;
    pushwork_ = new pushwork;
    initrtmp();


}
VideoWidget::~VideoWidget() {

}

void VideoWidget::init() {
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    // 设置背景为黑色
    QPalette palette = this->palette();
    palette.setColor(QPalette::Base, Qt::black);
    setPalette(palette);

    //初始化信号量和互斥量
    m_mutex = new QMutex;
    m_condition = new QWaitCondition;
    // 初始化状态
    m_videoReady = false;
    m_loadingText = tr("等待视频流打开中...");
}
void VideoWidget::startVideo() {
    videofromcamera->init();
    videofromcamera->setVideoWidget(this);
    setVideoReady(true);

    //开始显示线程
    runing = true;
    work_ = new std::thread(&VideoWidget::loop,this);
}

void VideoWidget::stopVideo() {
    setVideoReady(false);
    videofromcamera->requestExit();
}

// 更新视频流状态
void VideoWidget::setVideoReady(bool ready) {
    m_videoReady = ready;
    update(); // 触发重绘
}

int VideoWidget::initrtmp() {
    Properties  properties;
    properties.SetProperty("width", 640);
    properties.SetProperty("height", 480);
    properties.SetProperty("video_bitrate", 512 * 1024);  // 设置码率
    properties.SetProperty("fps", 30);
    properties.SetProperty("gop", 30);
    properties.SetProperty("video_b_frames", 0);

    if (pushwork_->initwork(properties) == -1) {
        std::cerr << "pushwork is fail" << std::endl;
        return -1;
    }
    return 0;
}
void VideoWidget::loop() {
    while (runing) {
        AVFrame* frame = nullptr;
        {
            QMutexLocker locker(m_mutex);
            while (m_frameQueue.empty() && runing)
                m_condition->wait(m_mutex);
            if (!runing) break;
            frame = m_frameQueue.front();
            m_frameQueue.pop_front();
        }

        if (!frame) continue;

        // 不再限定像素格式，直接给显示路径
        // （setAVFrameData 内部会把任何输入转成 RGB）
        setAVFrameData(frame);

        // 用完这份引用就释放（你 enqueue 用的是 av_frame_ref，所以这里 free 没问题）
        av_frame_free(&frame);
    }

    QMutexLocker locker(m_mutex);
    while (!m_frameQueue.empty()) { av_frame_free(&m_frameQueue.front()); m_frameQueue.pop_front(); }
}

void VideoWidget::enqueueFrame(const AVFrame *frame)
{
    if (!frame) {
        std::cerr << "Null frame enqueued" << std::endl;
        return;
    }

    QMutexLocker locker(m_mutex);

    // ------------------- 智能丢帧策略 -------------------
    // 根据队列长度动态调整丢帧率
    if (m_frameQueue.size() > DEQUE_MAX_SIZE * 0.7) {
        m_dropRate = 2;  // 队列占用70%以上，丢弃50%的帧
    } else if (m_frameQueue.size() > DEQUE_MAX_SIZE * 0.9) {
        m_dropRate = 3;  // 队列占用90%以上，丢弃66%的帧
    } else {
        m_dropRate = 1;  // 正常状态，不丢帧
    }

    // 按丢帧率丢弃帧
    m_frameCounter++;
    if (m_frameCounter % m_dropRate != 0) {
        std::cout << "Dropped frame, drop rate: " << m_dropRate << std::endl;
        return;
    }
    // ---------------------------------------------------

    // 队列已满时丢弃最旧的帧
    if (m_frameQueue.size() >= DEQUE_MAX_SIZE) {
        AVFrame* oldFrame = m_frameQueue.front();
        m_frameQueue.pop_front();
        av_frame_free(&oldFrame);
        std::cout << "Queue full, dropped oldest frame" << std::endl;
    }

    // 复制帧数据
    AVFrame *frameCopy = av_frame_alloc();
    if (!frameCopy) {
        std::cerr << "Failed to allocate frame copy" << std::endl;
        return;
    }

    // 复制帧属性
    av_frame_ref(frameCopy, const_cast<AVFrame*>(frame));

    // 加入队列
    m_frameQueue.push_back(frameCopy);
    m_condition->wakeOne();
}
//bug
void VideoWidget::setAVFrameData(const AVFrame* frame) {
    if (!frame || frame->width <= 0 || frame->height <= 0) return;

    const int w = frame->width;
    const int h = frame->height;

    // 目标：QImage 的 RGB24
    QImage img(w, h, QImage::Format_RGB888);
    if (img.isNull()) return;

    // 复用 sws 上下文，避免每帧创建
    static SwsContext* s_sws_to_rgb = nullptr;
    s_sws_to_rgb = sws_getCachedContext(
        s_sws_to_rgb,
        w, h, static_cast<AVPixelFormat>(frame->format),
        w, h, AV_PIX_FMT_RGB24,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    if (!s_sws_to_rgb) return;

    // ★ 颜色空间/范围 —— MJPEG 常见 601 + FULL(PC)
    //   这段即使源不是 FULL 也没副作用；想更严谨可以先判断 frame->color_range。
    sws_setColorspaceDetails(
        s_sws_to_rgb,
        sws_getCoefficients(SWS_CS_ITU601),  // 源矩阵
        AVCOL_RANGE_JPEG,                    // 源 FULL(PC)
        sws_getCoefficients(SWS_CS_ITU601),  // 目标矩阵
        AVCOL_RANGE_JPEG,                    // 目标按 FULL 处理到 RGB
        0, 1<<16, 1<<16
    );

    // 设置输出缓冲区
    uint8_t* dstData[4];
    dstData[0] = reinterpret_cast<uint8_t*>(img.bits());
    dstData[1] = nullptr;
    dstData[2] = nullptr;
    dstData[3] = nullptr;

    int dstLinesize[4];
    const qsizetype bpl = img.bytesPerLine();
    if (bpl > (std::numeric_limits<int>::max)()) {
        qWarning() << "bytesPerLine too large for int";
        return;
    }
    dstLinesize[0] = static_cast<int>(bpl);
    dstLinesize[1] = 0;
    dstLinesize[2] = 0;
    dstLinesize[3] = 0;

    // 执行像素转换：任何输入 → RGB24
    const int ret = sws_scale(
        s_sws_to_rgb,
        frame->data, frame->linesize,
        0, h,
        dstData, dstLinesize
    );
    if (ret <= 0) return;

    // 线程安全更新 + 在主线程触发重绘
    {
        QMutexLocker locker(m_mutex);
        m_image = std::move(img);
    }
    QMetaObject::invokeMethod(this, [this](){ update(); }, Qt::QueuedConnection);
}
// paintEvent方法保持不变
void VideoWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    if (!m_videoReady) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        // 设置文字颜色为白色
        QPen pen(Qt::white);
        painter.setPen(pen);

        // 设置字体（加粗，16pt）
        QFont font = painter.font();
        font.setPointSize(16);
        font.setBold(true);
        painter.setFont(font);

        // 在窗口中心绘制文本
        painter.drawText(rect(), Qt::AlignCenter, m_loadingText);
    }
    else {
        QPainter painter(this);
        QMutexLocker locker(m_mutex);
        if (!m_image.isNull())
        {
            // 保持比例绘制
            QRect targetRect = rect();
            QImage scaled = m_image.scaled(targetRect.size(), Qt::KeepAspectRatio);
            QPoint center = QPoint((width() - scaled.width()) / 2, (height() - scaled.height()) / 2);
            painter.drawImage(center, scaled);
        }
    }
}
