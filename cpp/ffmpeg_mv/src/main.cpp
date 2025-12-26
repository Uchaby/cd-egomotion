#include <iostream>
#include "ffmpeg_reader.hpp"
#include "mv_structs.hpp"

#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

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

static void draw_motion_vectors(
    cv::Mat& img,
    const std::vector<MotionVector>& mvs,
    int step = 1,
    float scale = 1.0f
) {
    for (size_t i = 0; i < mvs.size(); i += step) {
        const auto& mv = mvs[i];

        if (mv.motion_scale == 0)
            continue;

        int x0 = mv.dst_x;
        int y0 = mv.dst_y;

        int dx = (mv.motion_x * scale) / mv.motion_scale;
        int dy = (mv.motion_y * scale) / mv.motion_scale;

        int x1 = x0 + dx;
        int y1 = y0 + dy;

        // bounds check
        if (x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0 ||
            x0 >= img.cols || x1 >= img.cols ||
            y0 >= img.rows || y1 >= img.rows)
            continue;

        cv::Scalar color = mv.backward ? cv::Scalar(0, 0, 255) : cv::Scalar(0, 255, 0);

        cv::arrowedLine(
            img,
            cv::Point(x0, y0),
            cv::Point(x1, y1),
            color, // green
            1,
            cv::LINE_AA,
            0,
            0.3
        );
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

        reader.open();

        FrameData frame;

        int idx = 0;

        while (reader.read(frame)) {
            std::cout << "Frame " << idx++
                      << " type=" << frame.pict_type
                      << " mv=" << frame.motion_vectors.size()
                      << std::endl;

            cv::Mat img(
                frame.image.height,
                frame.image.width,
                CV_8UC3,
                frame.image.data,
                frame.image.stride
            );

            draw_motion_vectors(img, frame.motion_vectors, 1, 1.0f);

            cv::imshow("Decoded BGR", img);
            int key = cv::waitKey(1);
            if (key == 27) break; // ESC
            
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}