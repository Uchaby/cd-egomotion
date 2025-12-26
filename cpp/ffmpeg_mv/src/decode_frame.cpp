#include "decode_frame.hpp"

extern "C" {
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <stdexcept>

BGRImage decode_frame_to_bgr(const AVFrame* frame) {
    if (!frame)
        throw std::runtime_error("decode: Frame is NULL");

    BGRImage out;
    out.width = frame->width;
    out.height = frame->height;
    out.stride = frame->width*3;

    int bufsize = av_image_get_buffer_size(
        AV_PIX_FMT_BGR24,
        out.width, 
        out.height,
        1
    );

    out.data = static_cast<uint8_t*>(av_malloc(bufsize));
    if (!out.data)
        throw std::runtime_error("Failed to allocate BGR buffer");

    uint8_t* dst_data[4];
    int dst_linesize[4];

    av_image_fill_arrays(
        dst_data,
        dst_linesize,
        out.data,
        AV_PIX_FMT_BGR24,
        out.width,
        out.height,
        1
    );

    SwsContext* sws_ctx = sws_getContext(
        frame->width,
        frame->height,
        static_cast<AVPixelFormat>(frame->format),
        out.width,
        out.height,
        AV_PIX_FMT_BGR24,
        SWS_BILINEAR,
        nullptr,
        nullptr,
        nullptr
    );

    if (!sws_ctx)
        throw std::runtime_error("Failed to create SwsContext");

    sws_scale(
        sws_ctx,
        frame->data,
        frame->linesize,
        0,
        frame->height,
        dst_data,
        dst_linesize
    );

    sws_freeContext(sws_ctx);
    
    return out;
}

void free_bgr_image(BGRImage& img) {
    if (img.data) {
        av_free(img.data);
        img.data = nullptr;
    }

    //img.width = img.height = img.stride = 0;
}