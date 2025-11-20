//
// Created by Lenovo on 25-7-1.
//

#include "VideoFromCamera.h"
#include "./views/CameraView.h"  // 在实现文件中包含完整头文件

VideoFromCamera::VideoFromCamera(){}

VideoFromCamera::~VideoFromCamera(){}

int VideoFromCamera::init() {
    avdevice_register_all();

    const char *deviceName = "video=c922 Pro Stream Webcam";
    const AVInputFormat *inputformat = av_find_input_format("dshow");
    if (!inputformat) {
        std::cerr << "not find dshow format" << std::endl;
        return -1;
    }

    // 1) 分配 AVFormatContext，并在其上指定“期望的输入视频编码”
    ftx_ = avformat_alloc_context();
    if (!ftx_) {
        std::cerr << "avformat_alloc_context failed" << std::endl;
        return -1;
    }
    // ftx_->video_codec_id = AV_CODEC_ID_MJPEG;   // ★★ 核心：等价于 -vcodec mjpeg（对输入端）

    // 2) 常规参数仍用（这些在你这套库里会被吃到）
    AVDictionary *options = nullptr;
    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "framerate",  "30",      0);

    // av_log_set_level(AV_LOG_DEBUG);

    // 3) 打开输入（注意这里传的是 &ftx_，且 ftx_ 不是空指针）
    int ret = avformat_open_input(&ftx_, deviceName, inputformat, &options);
    if (ret < 0) {
        char errbuf[64] = {0};
        av_strerror(ret, errbuf, sizeof(errbuf));
        std::cerr << "open device is fail | error :" << ret << " | info:" << errbuf << std::endl;
        av_dict_free(&options);
        avformat_free_context(ftx_);
        ftx_ = nullptr;
        return -1;
    }

    // 4) 打印未被消费的选项（便于确认哪些键真正生效）
    if (options) {
        AVDictionaryEntry* t = nullptr;
        while ((t = av_dict_get(options, "", t, AV_DICT_IGNORE_SUFFIX))) {
            fprintf(stderr, "UNUSED OPTION: %s=%s\n", t->key, t->value);
        }
        av_dict_free(&options);
    }

    // 5) 读取流信息并打印最终协商
    if (avformat_find_stream_info(ftx_, nullptr) < 0) {
        std::cerr << "find stream info is fail" << std::endl;
        avformat_close_input(&ftx_);
        return -1;
    }
    av_dump_format(ftx_, 0, deviceName, 0); // ★ 这里应看到 Video: mjpeg, 640x480, 30 fps

    // 6) 查找视频流
    video_stream_index = -1;
    for (unsigned int i = 0; i < ftx_->nb_streams; ++i) {
        if (ftx_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            this->video_stream_index = static_cast<int>(i);
            AVCodecID codec_id = ftx_->streams[i]->codecpar->codec_id;
            std::cout << "Video: " << avcodec_get_name(codec_id) << " (ID: " << codec_id << ")\n";
            break;
        }
    }
    if (video_stream_index == -1) {
        std::cerr << "can not find video_stream_index" << std::endl;
        avformat_close_input(&ftx_);
        return -1;
    }

    // 7) 创建解码器
    pCodecParameters = ftx_->streams[video_stream_index]->codecpar;
    std::cout << "Codec ID: "   << pCodecParameters->codec_id << std::endl;
    std::cout << "Codec name: " << avcodec_get_name(pCodecParameters->codec_id) << std::endl;

    codec_ = avcodec_find_decoder(pCodecParameters->codec_id);
    if (!codec_) {
        std::cerr << "alloc codec_ is fail" << std::endl;
        avformat_close_input(&ftx_);
        return -1;
    }

    ctx_ = avcodec_alloc_context3(codec_);
    if (!ctx_) {
        std::cerr << "Could not allocate codec context." << std::endl;
        avformat_close_input(&ftx_);
        return -1;
    }
    if (avcodec_parameters_to_context(ctx_, pCodecParameters) < 0) {
        std::cerr << "alloc ctx is fail" << std::endl;
        avcodec_free_context(&ctx_);
        avformat_close_input(&ftx_);
        return -1;
    }
    if (avcodec_open2(ctx_, codec_, nullptr) < 0) {
        std::cerr << "avcodec_open2 is fail" << std::endl;
        avcodec_free_context(&ctx_);
        avformat_close_input(&ftx_);
        return -1;
    }

    frame = av_frame_alloc();
    if (!frame) {
        std::cerr << "av_frame_alloc is fail" << std::endl;
        avcodec_free_context(&ctx_);
        avformat_close_input(&ftx_);
        return -1;
    }

    request_exit_ = 0;
    work = new std::thread(&VideoFromCamera::Loop, this);
    return 1;
}

void VideoFromCamera::setVideoWidget(VideoWidget* widget) {
    videoWidget_ = widget;
}

void VideoFromCamera::Loop()//开新线程读取数据
{
    while (request_exit_ == 0){
        // std::cout<<this->video_stream_index << std::endl;
        //从摄像头中读取ACPacket帧
        int ret = av_read_frame(ftx_, &packet_);
        if (ret >= 0) {
            if (packet_.stream_index == video_stream_index) {
                //解码
                int ret = avcodec_send_packet(ctx_,&packet_);
                if (ret < 0) {
                    std::cerr << "error while sending a packet" << std::endl;
                    continue;
                }
                while (true) {
                    ret = avcodec_receive_frame(ctx_, frame);
                    if (ret == AVERROR(EAGAIN)) {
                        // 需要更多包才能输出帧，跳出内层循环
                        break;
                    } else if (ret == AVERROR_EOF) {
                        // 解码结束
                        request_exit_ = 1;
                        break;
                    } else if (ret < 0) {
                        std::cerr << "error receiving frame" << std::endl;
                        request_exit_ = 1;
                        break;
                    }
                    // //frame的格式
                    // auto fmt = (AVPixelFormat)frame->format;
                    // std::cout << "decoder output fmt = " << av_get_pix_fmt_name(fmt)
                    //     << ", range=" << av_color_range_name(frame->color_range) << "\n";
                    videoWidget_->pushwork_->YuvCallback(frame);
                    videoWidget_->enqueueFrame(frame);
                }

            }
            else {
                // 错误处理
                char errbuf[AV_ERROR_MAX_STRING_SIZE] = {0};
                av_strerror(ret, errbuf, sizeof(errbuf));
                std::cerr << "read is fail: " << errbuf << " (error: " << ret << ")" << std::endl;
            }
        }
        else {
            char errbuf[AV_ERROR_MAX_STRING_SIZE]  = {0};
            av_strerror(ret,errbuf,sizeof(errbuf));
            std::cerr << "av_read_frame is fail:" << errbuf << std::endl;
        }
        av_packet_unref(&packet_);
    }
}
void VideoFromCamera::requestExit() {
    request_exit_ = 1;
}

void VideoFromCamera::waitAndRelease() {
    if (work && work->joinable())
        work->join();            // 等线程函数自然返回

    if (frame)   av_frame_free(&frame);
    if (ctx_)    avcodec_free_context(&ctx_);
    if (ftx_)    avformat_close_input(&ftx_);

    delete work;
    work = nullptr;
}

