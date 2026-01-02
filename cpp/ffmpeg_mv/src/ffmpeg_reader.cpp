#include "ffmpeg_reader.hpp"

#include <cerrno>
#include <libavcodec/avcodec.h>
#include <libavutil/error.h>
#include <stdexcept>
#include <utility>

#include "decode_frame.hpp"
#include "extract_mv.hpp"
#include "mv_structs.hpp"


FFmpegReader::FFmpegReader(const std::string& path) {
        open(path);
        state_ = State::Opened;

    }

FFmpegReader::~FFmpegReader() {
    clean_up();
}

void FFmpegReader::clean_up() noexcept {
    if (packet_) av_packet_free(&packet_);
    if (frame_) av_frame_free(&frame_);
    if (codec_ctx_) avcodec_free_context(&codec_ctx_);
    if (fmt_ctx_) avformat_close_input(&fmt_ctx_);
    video_stream_idx_ = -1;
    state_ = State::Closed;
}

void FFmpegReader::ensure_opened() const {
    if (state_ != State::Opened) {
        throw std::runtime_error("FFmpegReader : not opened");
    }
}

bool FFmpegReader::isOpen() const noexcept {
    return state_ == State::Opened;
}

char FFmpegReader::pict_type_char(int pict_type) {
    switch (pict_type) {
        case AV_PICTURE_TYPE_I: return 'I';
        case AV_PICTURE_TYPE_P: return 'P';
        case AV_PICTURE_TYPE_B: return 'B';
        default: return '?';
    }
}

void FFmpegReader::open(const std::string& path) {
    if (state_ != State::Closed) {
        throw std::runtime_error("FFmpegReader::open: already opened or EOF (create new reader or add reopen later)");
    }

    int ret = 0;

    ret = avformat_open_input(&fmt_ctx_, path.c_str(), nullptr, nullptr);
    if (ret < 0) {
        clean_up();
        throw std::runtime_error("Failed to open input");
    }

    ret = avformat_find_stream_info(fmt_ctx_, nullptr);
    if (ret < 0) {
        clean_up();
        throw std::runtime_error("Failed to find stream info");
    }

    video_stream_idx_ = av_find_best_stream(fmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (video_stream_idx_ < 0) {
        clean_up();
        throw std::runtime_error("No video stream found");
    }

    AVStream* stream = fmt_ctx_->streams[video_stream_idx_];
    AVCodecParameters* codecpar = stream->codecpar;

    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        clean_up();
        throw std::runtime_error("Failed to find decoder");
    }

    codec_ctx_ = avcodec_alloc_context3(codec);
    if (!codec_ctx_) {
        clean_up();
        throw std::runtime_error("Failed to allocate codec context");
    }

    ret = avcodec_parameters_to_context(codec_ctx_, codecpar);
    if (ret < 0) {
        clean_up();
        throw std::runtime_error("Failed to copy codec parameters");
    }

    codec_ctx_->err_recognition = AV_EF_IGNORE_ERR;
    codec_ctx_->flags |= AV_CODEC_FLAG_OUTPUT_CORRUPT;

    codec_ctx_->flags2 |= AV_CODEC_FLAG2_EXPORT_MVS;

    ret = avcodec_open2(codec_ctx_, codec, nullptr);
    if (ret < 0) {
        clean_up();
        throw std::runtime_error("Failed to open codec");
    }

    frame_ = av_frame_alloc();
    packet_ = av_packet_alloc();
    if (!frame_ || !packet_) {
        clean_up();
        throw std::runtime_error("Failed to allocate frame or packet");
    }
}

bool FFmpegReader::flush_one() {
    // send NULL once is ok; ffmpeg will handle
    int ret = avcodec_send_packet(codec_ctx_, nullptr);
    if (ret < 0) {
        throw std::runtime_error("FFmpegReader::flush_one: avcodec_send_packet(NULL) failed");
    }

    ret = avcodec_receive_frame(codec_ctx_, frame_);
    if (ret == 0) return true;
    if (ret == AVERROR_EOF) return false;
    if (ret == AVERROR(EAGAIN)) return false;

    throw std::runtime_error("FFmpegReader::flush_one: avcodec_receive_frame failed");
}

bool FFmpegReader::decode_next() {
    ensure_opened();

    while (true) {
        int ret = av_read_frame(fmt_ctx_, packet_);
        if (ret < 0) {
            bool got = flush_one();
            if (!got) {
                state_ = State::Eof;
            }
            return got;
        }

        if (packet_->stream_index != video_stream_idx_) {
            av_packet_unref(packet_);
            continue;
        }

        ret = avcodec_send_packet(codec_ctx_, packet_);
        av_packet_unref(packet_);

        if (ret == AVERROR(EAGAIN)) {}
        else if (ret == AVERROR_INVALIDDATA) {
            continue;
        }
        else if (ret < 0) {
            state_ = State::Eof;
            return false;
        }

        ret = avcodec_receive_frame(codec_ctx_, frame_);

        if (ret == 0) {
            return true;
        }

        if (ret == AVERROR(EAGAIN)) {
            continue;
        }

        if (ret == AVERROR_INVALIDDATA) {
            state_ = State::Eof;
            return false;
        }

        state_ = State::Eof;
        return false;

    }
}

FrameData FFmpegReader::read() {
    FrameData out;

    if (state_ != State::Opened) {
        out.ok = false;
        return out;
    }

    if (!decode_next()) {
        state_ = State::Eof;
        out.ok = false;
        return out;
    }

    out.ok = true;
    out.pts = frame_->pts;

    switch (frame_->pict_type) {
        case AV_PICTURE_TYPE_I: out.pict_type = 'I'; break;
        case AV_PICTURE_TYPE_P: out.pict_type = 'P'; break;
        case AV_PICTURE_TYPE_B: out.pict_type = 'B'; break;
        default: out.pict_type = '?'; break;
    }

    out.motion_vectors = extract_mv(*frame_);
    out.image = decode_frame_to_bgr(*frame_);

    return out;
}