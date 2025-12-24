#pragma once
#include <string>
#include <vector>
#include "mv_structs.hpp"
extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
}

class FFmpegReader {
public:
    explicit FFmpegReader(const std::string& path);
    ~FFmpegReader();

    bool open();
    bool read_frame();
    std::vector<MotionVector> extract_motion_vectors();
    AVFrame* frame() const { return frame_; }

private:
    std::string path_;

    struct AVFormatContext*     fmt_ctx_ = nullptr;
    struct AVCodecContext*      codec_ctx_ = nullptr;
    struct AVFrame*             frame_ = nullptr;
    struct AVPacket*            packet_ = nullptr;

    int video_stream_idx_ = -1;

};