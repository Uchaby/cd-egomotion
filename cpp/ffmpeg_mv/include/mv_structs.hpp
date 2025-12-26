#pragma once
#include <cstdint>
#include "decode_frame.hpp"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/motion_vector.h>
}

struct MotionVector {
    int32_t src_x;
    int32_t src_y;
    int32_t dst_x;
    int32_t dst_y;

    int32_t motion_x;
    int32_t motion_y;

    int32_t block_w;
    int32_t block_h;

    int32_t source;
    int32_t frame_idx;

    uint16_t motion_scale;
    uint64_t flags;

    bool backward;
};

struct FrameData {
    int64_t pts = 0;
    char pict_type = '?';

    BGRImage image;

    std::vector<MotionVector> motion_vectors;

    AVFrame* frame = nullptr;
};
