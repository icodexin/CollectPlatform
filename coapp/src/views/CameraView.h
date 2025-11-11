#ifndef CAMERAVIEW_H
#define CAMERAVIEW_H

#include "components/BarCard.h"
#include "./components/VideoRealTime/VideoFromCamera.h"
#include<QPainter>
#include<QMutex>
#include "./components/VideoRealTime/pushwork.h"
#include "./components/VideoRealTime/mediabase.h"
#include <thread>
#include<deque>
#include <QWaitCondition>
#include<iostream>
extern "C"
{
    #include "libavdevice/avdevice.h"
    #include "libavcodec/avcodec.h"
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}
#include <QMetaObject>      // for QMetaObject::invokeMethod

#define DEQUE_MAX_SIZE 30
class VideoWidget : public QWidget {
    Q_OBJECT
public:
    VideoWidget(QWidget *parent = NULL);
    ~VideoWidget();
    void setAVFrameData(const AVFrame* frame);
    //初始化ui
    void init();
    void setVideoReady(bool ready);
    int initrtmp();//初始化编码器和rtmp流
    pushwork *pushwork_ = NULL;//真正传输数据的类
    void enqueueFrame(const AVFrame *frame);
    void loop();
private:
    VideoFromCamera *videofromcamera = NULL;
    QImage *image_ = NULL;
    bool m_videoReady = false;
    QString m_loadingText;
    std::thread *work_ = NULL;
protected:
    void paintEvent(QPaintEvent *event);
private:
    std::deque<AVFrame *> m_frameQueue;
    QImage m_image;
    bool runing = false;
    //保证输出视频帧队列始终只有一个进程访问
    QMutex *m_mutex;
    QWaitCondition *m_condition;

    // 新增丢帧相关变量
    int m_dropRate = 2;         // 丢帧率，1表示不丢帧，2表示丢50%，依此类推
    int m_frameCounter = 0;     // 帧计数器

public slots:
    void startVideo();
    void stopVideo();
};
#endif //CAMERAVIEW_H
