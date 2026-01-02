#pragma once
#include <string>
#include <vector>
#include "mv_structs.hpp"
extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/packet.h>
}


class FFmpegReader {
public:
    explicit FFmpegReader(const std::string& path);
    ~FFmpegReader();

    FFmpegReader(const FFmpegReader&) = delete;
    FFmpegReader& operator=(const FFmpegReader&) = delete;
    FFmpegReader(FFmpegReader&&) = delete;
    FFmpegReader& operator=(FFmpegReader&&) = delete;

    bool isOpen() const noexcept;
    FrameData read();

private:
    enum class State { Closed, Opened, Eof };

    bool decode_next();
    bool flush_one();
    void open(const std::string& path);

    static char pict_type_char(int pict_type);

    std::string path_;
    State state_ = State::Closed;

    AVFormatContext* fmt_ctx_ = nullptr;
    AVCodecContext*  codec_ctx_ = nullptr;
    AVFrame*         frame_ = nullptr;
    AVPacket*        packet_ = nullptr;

    int video_stream_idx_ = -1;

    void ensure_opened() const;
    void clean_up() noexcept;
};