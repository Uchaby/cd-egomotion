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
    bool is_open() const;
    bool read(FrameData& out);
    AVFrame* frame() const { return frame_; }

private:
    std::string path_;

    bool decode_next();
    std::vector<MotionVector> extract_mv();

    struct AVFormatContext*     fmt_ctx_ = nullptr;
    struct AVCodecContext*      codec_ctx_ = nullptr;
    struct AVFrame*             frame_ = nullptr;
    struct AVPacket*            packet_ = nullptr;

    int video_stream_idx_ = -1;

};