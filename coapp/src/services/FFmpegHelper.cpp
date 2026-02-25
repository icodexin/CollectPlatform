#include "FFmpegHelper.h"

#include <QDebug>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
}

namespace {
    constexpr FFEncoderInfo kAllCodecs[] = {
        {FF_ET_X264, FF_EWT_WARE_SW, "libx264", "H.264 (x264, Software)"},
        {FF_ET_OpenH264, FF_EWT_WARE_SW, "libopenh264", "H.264 (OpenH264, Software)"},
        {FF_ET_H264_VideoToolbox, FF_EWT_WARE_HW, "h264_videotoolbox", "H.264 (Apple VideoToolbox, Hardware)"},
        {FF_ET_H264_QSV, FF_EWT_WARE_HW, "h264_qsv", "H.264 (Intel QSV, Hardware)"},
        {FF_ET_H264_NVENC, FF_EWT_WARE_HW, "h264_nvenc", "H.264 (NVIDIA NVENC, Hardware)"},
    };
}

namespace FFMpegHelper {
    AVColorRange toAvColorRange(const QVideoFrameFormat::ColorRange r) {
        switch (r) {
            case QVideoFrameFormat::ColorRange_Video: return AVCOL_RANGE_MPEG;
            case QVideoFrameFormat::ColorRange_Full: return AVCOL_RANGE_JPEG;
            default: return AVCOL_RANGE_UNSPECIFIED;
        }
    }

    AVColorSpace toAvColorSpace(const QVideoFrameFormat::ColorSpace cs) {
        switch (cs) {
            case QVideoFrameFormat::ColorSpace_BT601: return AVCOL_SPC_BT470BG;
            case QVideoFrameFormat::ColorSpace_BT709: return AVCOL_SPC_BT709;
            case QVideoFrameFormat::ColorSpace_AdobeRgb: return AVCOL_SPC_RGB;
            case QVideoFrameFormat::ColorSpace_BT2020: return AVCOL_SPC_BT2020_CL;
            default: return AVCOL_SPC_UNSPECIFIED;
        }
    }

    AVColorTransferCharacteristic toAvColorTrc(const QVideoFrameFormat::ColorTransfer trc) {
        switch (trc) {
            case QVideoFrameFormat::ColorTransfer_BT709: return AVCOL_TRC_BT709;
            case QVideoFrameFormat::ColorTransfer_BT601: return AVCOL_TRC_SMPTE170M;
            case QVideoFrameFormat::ColorTransfer_Linear: return AVCOL_TRC_LINEAR;
            case QVideoFrameFormat::ColorTransfer_Gamma22: return AVCOL_TRC_GAMMA22;
            case QVideoFrameFormat::ColorTransfer_Gamma28: return AVCOL_TRC_GAMMA28;
            case QVideoFrameFormat::ColorTransfer_ST2084: return AVCOL_TRC_SMPTE2084;
            case QVideoFrameFormat::ColorTransfer_STD_B67: return AVCOL_TRC_ARIB_STD_B67;
            default: return AVCOL_TRC_UNSPECIFIED;
        }
    }

    AVPixelFormat toAvPixelFormat(QVideoFrameFormat::PixelFormat fmt) {
        switch (fmt) {
            default:
            case QVideoFrameFormat::Format_Invalid:
            case QVideoFrameFormat::Format_AYUV:
            case QVideoFrameFormat::Format_AYUV_Premultiplied:
            case QVideoFrameFormat::Format_YV12:
            case QVideoFrameFormat::Format_IMC1:
            case QVideoFrameFormat::Format_IMC2:
            case QVideoFrameFormat::Format_IMC3:
            case QVideoFrameFormat::Format_IMC4:
                return AV_PIX_FMT_NONE;
            case QVideoFrameFormat::Format_Jpeg:
                // We're using the data from the converted QImage here, which is in BGRA.
                return AV_PIX_FMT_BGRA;
            case QVideoFrameFormat::Format_ARGB8888:
                return AV_PIX_FMT_ARGB;
            case QVideoFrameFormat::Format_ARGB8888_Premultiplied:
            case QVideoFrameFormat::Format_XRGB8888:
                return AV_PIX_FMT_0RGB;
            case QVideoFrameFormat::Format_BGRA8888:
                return AV_PIX_FMT_BGRA;
            case QVideoFrameFormat::Format_BGRA8888_Premultiplied:
            case QVideoFrameFormat::Format_BGRX8888:
                return AV_PIX_FMT_BGR0;
            case QVideoFrameFormat::Format_ABGR8888:
                return AV_PIX_FMT_ABGR;
            case QVideoFrameFormat::Format_XBGR8888:
                return AV_PIX_FMT_0BGR;
            case QVideoFrameFormat::Format_RGBA8888:
                return AV_PIX_FMT_RGBA;
            // to be added in 6.8:
            // case QVideoFrameFormat::Format_RGBA8888_Premultiplied:
            case QVideoFrameFormat::Format_RGBX8888:
                return AV_PIX_FMT_RGB0;

            case QVideoFrameFormat::Format_YUV422P:
                return AV_PIX_FMT_YUV422P;
            case QVideoFrameFormat::Format_YUV420P:
                return AV_PIX_FMT_YUV420P;
            case QVideoFrameFormat::Format_YUV420P10:
                return AV_PIX_FMT_YUV420P10;
            case QVideoFrameFormat::Format_UYVY:
                return AV_PIX_FMT_UYVY422;
            case QVideoFrameFormat::Format_YUYV:
                return AV_PIX_FMT_YUYV422;
            case QVideoFrameFormat::Format_NV12:
                return AV_PIX_FMT_NV12;
            case QVideoFrameFormat::Format_NV21:
                return AV_PIX_FMT_NV21;
            case QVideoFrameFormat::Format_Y8:
                return AV_PIX_FMT_GRAY8;
            case QVideoFrameFormat::Format_Y16:
                return AV_PIX_FMT_GRAY16;

            case QVideoFrameFormat::Format_P010:
                return AV_PIX_FMT_P010;
            case QVideoFrameFormat::Format_P016:
                return AV_PIX_FMT_P016;

            case QVideoFrameFormat::Format_SamplerExternalOES:
                return AV_PIX_FMT_MEDIACODEC;
        }
    }

    const char* getCodecName(const FFEncoderType type) {
        for (const auto& c : kAllCodecs) {
            if (c.type == type) {
                return c.name;
            }
        }
        return nullptr;
    }

    const char* getCodecDisplay(const FFEncoderType type) {
        for (const auto& c : kAllCodecs) {
            if (c.type == type) {
                return c.display;
            }
        }
        return nullptr;
    }

    const char* getProfileName(const FFProfile profile) {
        switch (profile) {
            case FF_PROFILE_BASELINE: return "Baseline";
            case FF_PROFILE_MAIN: return "Main";
            case FF_PROFILE_HIGH: return "High";
            default: return "Unknown";
        }
    }

    QList<FFProfileOption> getProfileOptions() {
        return {
            {getProfileName(FF_PROFILE_BASELINE), FF_PROFILE_BASELINE},
            {getProfileName(FF_PROFILE_MAIN), FF_PROFILE_MAIN},
            {getProfileName(FF_PROFILE_HIGH), FF_PROFILE_HIGH},
        };
    }

    const AVCodec* findEncoder(const FFEncoderType type) {
        if (const char* name = getCodecName(type)) {
            return avcodec_find_encoder_by_name(name);
        }
        return nullptr;
    }

    QList<AVPixelFormat> getSupportedPixFmts(const AVCodec* codec) {
        QList<AVPixelFormat> result;
        if (codec) {
            const AVPixelFormat* pix_fmts = nullptr;
            int count = 0;
            avcodec_get_supported_config(
                nullptr, codec, AV_CODEC_CONFIG_PIX_FORMAT, 0,
                reinterpret_cast<const void**>(&pix_fmts), &count);
            for (int i = 0; i < count; ++i) {
                result.append(pix_fmts[i]);
            }
        }
        return result;
    }

    QList<AVPixelFormat> getSupportedPixFmts(const FFEncoderType type) {
        return getSupportedPixFmts(findEncoder(type));
    }

    QList<FFEncoderInfo> getAvailableEncoders() {
        QList<FFEncoderInfo> result;
        // 静音 ffmpeg 日志，避免探测时产生大量输出
        av_log_set_level(AV_LOG_QUIET);
        for (const auto& c : kAllCodecs) {
            if (const AVCodec* codec = avcodec_find_encoder_by_name(c.name)) {
                // 尝试以最小参数打开，验证硬件是否真实可用
                AVCodecContext* ctx = avcodec_alloc_context3(codec);
                if (ctx) {
                    ctx->width = 320;
                    ctx->height = 240;
                    ctx->time_base = {1, 30};
                    ctx->framerate = {30, 1};
                    ctx->pix_fmt = getSupportedPixFmts(codec).first(); // 选第一个支持的像素格式
                    if (avcodec_open2(ctx, codec, nullptr) == 0) {
                        result.append(c);
                    }
                    avcodec_free_context(&ctx);
                }
            }
        }
        av_log_set_level(AV_LOG_WARNING);
        return result;
    }

    QList<AVPixelFormat> getPreferredPixFmts(const FFEncoderType type) {
        switch (type) {
            case FF_ET_X264:
            case FF_ET_H264_VideoToolbox:
            case FF_ET_H264_QSV:
            case FF_ET_H264_NVENC:
                return {AV_PIX_FMT_YUV420P, AV_PIX_FMT_NV12};
            default:
                return {AV_PIX_FMT_YUV420P};
        }
    }
}
