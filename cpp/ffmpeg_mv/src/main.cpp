#include <iostream>
#include "ffmpeg_reader.hpp"
extern "C" {
#include <libavutil/frame.h>
}

static char pict_type_to_char(AVPictureType t) {
    switch (t) {
        case AV_PICTURE_TYPE_I: return 'I';
        case AV_PICTURE_TYPE_P: return 'P';
        case AV_PICTURE_TYPE_B: return 'B';
        default: return '?';
    }
}


int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: ffmpeg_mv <video_path>\n";
        return 1;
    }

    const std::string video_path = argv[1];

    try {
        FFmpegReader reader(video_path);

        if (!reader.open()) {
            std::cerr << "Failed to open video\n";
            return 1;
        }

        int frame_idx = 0;
        
        while (reader.read_frame()) {
            auto mvs = reader.extract_motion_vectors();

            std::cout << "Frame: " << frame_idx++
                      << " | typo: " << pict_type_to_char(reader.frame()->pict_type)
                      << " | motion vectors: " << mvs.size()
                      << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}