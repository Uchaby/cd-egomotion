#include "extract_mv.hpp"

extern "C" {
#include <libavutil/motion_vector.h>
}

std::vector<MotionVector> extract_mv(const AVFrame& frame) {
    std::vector<MotionVector> result;

    AVFrameSideData* sd = av_frame_get_side_data(
        const_cast<AVFrame*>(&frame), AV_FRAME_DATA_MOTION_VECTORS
    );
    if (!sd || !sd->data) return result;

    const auto* mvs = reinterpret_cast<const AVMotionVector*>(sd->data);
    const int count = sd->size / sizeof(AVMotionVector);

    result.reserve(count);

    for (int i = 0; i < count; ++i) {
        const AVMotionVector& mv = mvs[i];
        MotionVector out{};
        out.src_x = mv.src_x;
        out.src_y = mv.src_y;
        out.dst_x = mv.dst_x;
        out.dst_y = mv.dst_y;
        out.motion_x = mv.motion_x;
        out.motion_y = mv.motion_y;
        out.block_w = mv.w;
        out.block_h = mv.h;
        out.source = mv.source;
        out.motion_scale = mv.motion_scale;
        //out.flags = mv.flags;
        result.push_back(out);
    }

    return result;
}
