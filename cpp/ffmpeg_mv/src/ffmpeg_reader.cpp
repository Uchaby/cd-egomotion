extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/motion_vector.h>
}

#include "ffmpeg_reader.hpp"
#include <stdexcept>
#include <iostream>

FFmpegReader::FFmpegReader(const std::string& path)
    : path_(path) {}

FFmpegReader::~FFmpegReader() {
    if (packet_) av_packet_free(&packet_);
    if (frame_) av_frame_free(&frame_);
    if (codec_ctx_) avcodec_free_context(&codec_ctx_);
    if (fmt_ctx_) avformat_close_input(&fmt_ctx_); 
}

bool FFmpegReader::open() {
    int ret = 0;

    // 1. Open input
    ret = avformat_open_input(&fmt_ctx_, path_.c_str(), nullptr, nullptr);
    if (ret < 0) {
        throw std::runtime_error("Failed to open input");
    }

    // 2. Read stream info
    ret = avformat_find_stream_info(fmt_ctx_, nullptr);
    if (ret < 0) {
        throw std::runtime_error("Failed to find stream info");
    }

    // 3. Find best video stream
    video_stream_idx_ = av_find_best_stream(
        fmt_ctx_,
        AVMEDIA_TYPE_VIDEO,
        -1,
        -1,
        nullptr,
        0
    );

    if (video_stream_idx_ < 0) {
        throw std::runtime_error("No video stream found");
    }

    AVStream* stream = fmt_ctx_->streams[video_stream_idx_];
    AVCodecParameters* codecpar = stream->codecpar;

    std::cout << "Codec ID: "
          << avcodec_get_name(codecpar->codec_id)
          << std::endl;

    // 4. Find decoder
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        throw std::runtime_error("Failed to find decoder");
    }

    // 5. Allocate codec context
    codec_ctx_ = avcodec_alloc_context3(codec);
    if (!codec_ctx_) {
        throw std::runtime_error("Failed to allocate codec context");
    }

    // 6. Copy codec parameters
    ret = avcodec_parameters_to_context(codec_ctx_, codecpar);
    if (ret < 0) {
        throw std::runtime_error("Failed to copy codec parameters");
    }

    // 7. IMPORTANT: enable motion vector export
    codec_ctx_->flags2 |= AV_CODEC_FLAG2_EXPORT_MVS;

    // 8. Open decoder
    ret = avcodec_open2(codec_ctx_, codec, nullptr);
    if (ret < 0) {
        throw std::runtime_error("Failed to open codec");
    }

    // 9. Allocate frame and packet
    frame_ = av_frame_alloc();
    packet_ = av_packet_alloc();

    if (!frame_ || !packet_) {
        throw std::runtime_error("Failed to allocate frame or packet");
    }

    return true;
}

bool FFmpegReader::read_frame() {
    while (av_read_frame(fmt_ctx_, packet_) >= 0) {

        if (packet_->stream_index != video_stream_idx_) {
            av_packet_unref(packet_);
            continue;
        }

        int ret = avcodec_send_packet(codec_ctx_, packet_);
        av_packet_unref(packet_);

        if (ret < 0) {
            return false;
        }

        ret = avcodec_receive_frame(codec_ctx_, frame_);
        if (ret == AVERROR(EAGAIN)) {
            continue;
        }
        if (ret < 0) {
            return false;
        }

        return true; // получили кадр
    }

    // flush decoder
    avcodec_send_packet(codec_ctx_, nullptr);
    if (avcodec_receive_frame(codec_ctx_, frame_) == 0) {
        return true;
    }

    return false; // EOF
}


// bool FFmpegReader::read_frame() {
//     while (true) {
//         int ret = av_read_frame(fmt_ctx_, packet_);
//         if (ret < 0) {
//             return false;
//         }

//         if (packet_->stream_index != video_stream_idx_) {
//             av_packet_unref(packet_);
//             continue;
//         }

//         ret = avcodec_send_packet(codec_ctx_, packet_);
//         av_packet_unref(packet_);
        
//         if (ret < 0) {
//             continue;
//         }

//         ret = avcodec_receive_frame(codec_ctx_, frame_);
//         if (ret=AVERROR(EAGAIN)) {
//             continue;
//         }
//         if (ret < 0) {
//             return false;
//         }

//         return true;
//     }
// }