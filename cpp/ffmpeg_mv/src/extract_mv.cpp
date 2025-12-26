// extern "C" {
// #include <libavformat/avformat.h>
// #include <libavcodec/avcodec.h>
// #include <libavutil/frame.h>
// #include <libavutil/motion_vector.h>
// }

#include "ffmpeg_reader.hpp"

#include <iostream>

std::vector<MotionVector> FFmpegReader::extract_mv() {
    std::vector<MotionVector> result;

    AVFrameSideData* sd = av_frame_get_side_data(frame_, AV_FRAME_DATA_MOTION_VECTORS);

    if (!sd)
        return result;

    auto* mvs = (const AVMotionVector*)sd->data;
    int count = sd->size / sizeof(AVMotionVector);

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
        out.backward = mv.source < 0;

        out.motion_scale = mv.motion_scale;
        out.flags = mv.flags;

        result.push_back(out);
    }
    
    return result;
}