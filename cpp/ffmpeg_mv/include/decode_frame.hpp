#pragma once
#include <cstring>

extern "C" {
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
}

struct BGRImage {
    uint8_t* data = nullptr;
    int width = 0;
    int height = 0;
    int stride = 0;

    ~BGRImage() { if (data) av_free(data); }

    BGRImage() = default;

    BGRImage(const BGRImage&) = delete;
    BGRImage& operator=(const BGRImage&) = delete;

    BGRImage(BGRImage&& other) noexcept { *this = std::move(other); }
    BGRImage& operator=(BGRImage&& other) noexcept {
        if (this != &other) {
            if (data) av_free(data);
            data = other.data; other.data = nullptr;
            width = other.width; height = other.height; stride = other.stride;
        }
        return *this;
    }
};

BGRImage decode_frame_to_bgr(const AVFrame* frame);

void free_bgr_image(BGRImage& img);