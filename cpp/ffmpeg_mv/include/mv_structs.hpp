#pragma once
#include <cstdint>

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
    bool backward;
};
