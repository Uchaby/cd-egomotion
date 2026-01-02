#pragma once
#include <vector>
#include "mv_structs.hpp"

extern "C" {
#include <libavutil/frame.h>
}

std::vector<MotionVector> extract_mv(const AVFrame& frame);
