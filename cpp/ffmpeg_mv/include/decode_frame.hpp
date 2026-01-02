#pragma once
#include <cstdint>

extern "C" {
#include <libavutil/mem.h>
#include <libavutil/frame.h>
}

struct BGRImage {
    uint8_t* data = nullptr;
    int width = 0;
    int height = 0;
    int stride = 0;

    BGRImage() = default;

    ~BGRImage() { reset(); }

    BGRImage(const BGRImage&) = delete;
    BGRImage& operator=(const BGRImage&) = delete;

    BGRImage(BGRImage&& other) noexcept { *this = std::move(other); }

    BGRImage& operator=(BGRImage&& other) noexcept {
        if (this != &other) {
            reset();
            data = other.data;
            width = other.width;
            height = other.height;
            stride = other.stride;
            other.data = nullptr;
            other.width = other.height = other.stride = 0;
        }
        return *this;
    }

    void reset() noexcept {
        if (data) {
            av_free(data);
            data = nullptr;
        }
    }
};

BGRImage decode_frame_to_bgr(const AVFrame& frame);
