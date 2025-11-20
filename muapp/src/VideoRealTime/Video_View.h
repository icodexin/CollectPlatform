//
// Created by Lenovo on 25-7-16.
//

#ifndef VIDEO_VIEW_H
#define VIDEO_VIEW_H
#include <functional>
#include <QImage>
#include <QTimer>
#include <QMutex>
#include <QWaitCondition>
#include<thread>
#include<mutex>
#include<condition_variable>
#include<iostream>
#include "framequeue.h"
#include "mediabase.h"
#include<QBuffer>
#include<QMetaObject>
#include<QDateTime>
#include<QThread>
extern "C"
{
    #include <libswscale/swscale.h>  // FFmpeg的格式转换库
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libavutil/opt.h"
    #include "libavutil/channel_layout.h"
}


class Video_View : public QObject
{
    Q_OBJECT
public:
    Video_View();
    ~Video_View();
    int init(FrameQueue *farmeq,const Properties video_info);
public:
    void addQimageUse(std::function<void(QImage)> callback) {
        callback_ = callback;
    }
    int Start();
private:
    std::function<void(QImage)> callback_;
    void Loop();
private:
    FrameQueue *pkq_ = NULL;
    SwsContext* sws_ctx = nullptr;  // 用于YUV转RGB的上下文
    AVFrame *frame_ = NULL;
    std::mutex *mutex_ = NULL;
    std::condition_variable *cond = NULL;
    std::thread *work_ = NULL;
    uint8_t* rgb_data = NULL; // RGB32每像素4字节
    QImage current_image_;
    int width = 0;
    int height = 0;

    int rgb_buffer_size = 640 * 480 * 4;  // 对于RGB32格式
};



#endif //VIDEO_VIEW_H
