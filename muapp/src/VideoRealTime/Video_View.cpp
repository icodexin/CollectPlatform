//
// Created by Lenovo on 25-7-16.
//

#include "Video_View.h"

Video_View::Video_View()
{
}
int Video_View::init(FrameQueue *farmeq,const Properties video_info) {
    pkq_ = farmeq;

    width = video_info.GetProperty("width",640);
    height = video_info.GetProperty("height",480);

    // 初始化SWS上下文（用于YUV420P转RGB）
    sws_ctx = sws_getContext(
        width, height, AV_PIX_FMT_YUV420P,
        width, height, AV_PIX_FMT_RGB32,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    if (sws_ctx == NULL) {
        std::cerr << "alloc sws_ctx is error" << std::endl;
        return -1;
    }
    frame_ = av_frame_alloc();
    if (frame_ == NULL) {
        std::cerr << "alloc frame is error" << std::endl;
        return -1;
    }
    mutex_ = new std::mutex;
    if (mutex_ == NULL) {
        std::cerr << "alloc mutex_ is error" << std::endl;
        return -1;
    }

    cond = new std::condition_variable;
    if (cond == NULL) {
        std::cerr << "alloc cond is error" << std::endl;
        return -1;
    }
    rgb_data = new uint8_t[1080 * 720 * 4];//先分配内存
    return 0;
}
Video_View::~Video_View() {
}
int Video_View::Start() {
    work_ = new std::thread(&Video_View::Loop,this);
    if (work_ == NULL) {
        std::cerr << "new thread is error" << std::endl;
        return -1;
    }
    return 0;
}

void Video_View::Loop() {
    // 添加帧率控制变量
    static qint64 lastProcessTime = 0;

    while (1) {
        if (pkq_->abort_request_ == 1)
            return ;

        // 控制处理速度，避免过度消耗CPU
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        //修改这个位置
        // if (currentTime - lastProcessTime < 25) {
        //     QThread::msleep(1);
        //     continue;
        // }
        lastProcessTime = currentTime;

        if (pkq_->frame_queue_peek_readable() == NULL) continue;
        frame_ = pkq_->frame_queue_peek_readable()->frame;

        // 检查是否需要调整尺寸（如果源尺寸与目标尺寸不同）
        if (frame_->width != width || frame_->height != height) {
            // 重新初始化转换器
            if (sws_ctx) {
                sws_freeContext(sws_ctx);
            }
            width = frame_->width;
            height = frame_->height;
            sws_ctx = sws_getContext(width, height, (AVPixelFormat)frame_->format,
                                    width, height, AV_PIX_FMT_RGB32,
                                    SWS_FAST_BILINEAR, NULL, NULL, NULL);
        }
        // // 优化内存分配 - 使用预分配的缓冲区而非每次new
        // if (!rgb_data || width * height * 4 != rgb_buffer_size) {
        //     if (rgb_data) delete[] rgb_data;
        //     rgb_buffer_size = width * height * 4;
        //     rgb_data = new uint8_t[rgb_buffer_size];
        // }

        // 设置目标缓冲区
        uint8_t* dest[4] = { rgb_data, nullptr, nullptr, nullptr };
        int dest_linesize[4] = { width * 4, 0, 0, 0 };

        // 执行转换
        sws_scale(sws_ctx,
                 frame_->data, frame_->linesize, 0, height,
                 dest, dest_linesize);

        // 创建QImage（使用预分配的缓冲区）
        QImage temp_image(rgb_data, width, height, QImage::Format_RGB32);

        // 深拷贝图像，确保线程安全
        QImage deep_copy;
        {
            std::lock_guard<std::mutex> lock(*mutex_);
            deep_copy = temp_image.copy(); // 深拷贝
        }

        // 使用排队连接调用回调，避免阻塞解码线程
        // QMetaObject::invokeMethod(this, [this, deep_copy]() {
        //     callback_(deep_copy);
        // }, Qt::QueuedConnection);
        callback_(deep_copy);

        pkq_->frame_queue_next();
    }
}

