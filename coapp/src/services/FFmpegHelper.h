#ifndef FFMPEGHELPER_H
#define FFMPEGHELPER_H

extern "C" {
#include <libavcodec/defs.h>
#include <libavformat/avformat.h>
}

#include <QtCore/QList>
#include <QtMultimedia/QVideoFrameFormat>

struct AVCodec;

/// 编码器类型
enum FFEncoderType {
    FF_ET_X264,                 // libx264, Software (GPL License)
    FF_ET_OpenH264,             // Cisco OpenH264, Software
    FF_ET_H264_VideoToolbox,    // Apple VideoToolbox, Hardware (macOS / iOS only)
    FF_ET_H264_QSV,             // Intel Quick Sync Video, Hardware (requires Intel GPU)
    FF_ET_H264_NVENC,           // NVIDIA NVENC, Hardware (requires NVIDIA GPU)
};

/// 编码器所属软件/硬件类别
enum FFEncoderWareType {
    FF_EWT_WARE_SW, // Software codec
    FF_EWT_WARE_HW, // Hardware codec
};

/// 编码配置
enum FFProfile {
    FF_PROFILE_BASELINE=AV_PROFILE_H264_BASELINE,
    FF_PROFILE_MAIN=AV_PROFILE_H264_MAIN,
    FF_PROFILE_HIGH=AV_PROFILE_H264_HIGH,
};

using FFProfileOption = std::pair<const char *, int>;

/// 编码器信息
struct FFEncoderInfo {
    FFEncoderType type;
    FFEncoderWareType wareType;
    const char *name;
    const char *display;
};

namespace FFMpegHelper {
    AVColorRange toAvColorRange(QVideoFrameFormat::ColorRange r);

    AVColorSpace toAvColorSpace(QVideoFrameFormat::ColorSpace cs);

    AVColorTransferCharacteristic toAvColorTrc(QVideoFrameFormat::ColorTransfer trc);

    AVPixelFormat toAvPixelFormat(QVideoFrameFormat::PixelFormat fmt);

    const char* getCodecName(FFEncoderType type);

    const char* getCodecDisplay(FFEncoderType type);

    const char* getProfileName(FFProfile profile);

    QList<FFProfileOption> getProfileOptions();

    const AVCodec* findEncoder(FFEncoderType type);

    QList<FFEncoderInfo> getAvailableEncoders();

    QList<AVPixelFormat> getPreferredPixFmts(FFEncoderType type);
}

#endif //FFMPEGHELPER_H
