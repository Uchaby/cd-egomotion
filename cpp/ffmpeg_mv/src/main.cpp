#include "ffmpeg_reader.hpp"
#include "mv_structs.hpp"

#include <iostream>

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/highgui.hpp>
#include <opencv4/opencv2/imgproc.hpp>

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

        const int x0 = mv.dst_x;
        const int y0 = mv.dst_y;

        const int dx = static_cast<int>((mv.motion_x * scale) / mv.motion_scale);
        const int dy = static_cast<int>((mv.motion_y * scale) / mv.motion_scale);

        const int x1 = x0 + dx;
        const int y1 = y0 + dy;

        // bounds check
        if (x0 < 0 || y0 < 0 || x1 < 0 || y1 < 0 ||
            x0 >= img.cols || x1 >= img.cols ||
            y0 >= img.rows || y1 >= img.rows)
            continue;

        const cv::Scalar color =
            (mv.source < 0) ? cv::Scalar(0, 0, 255)   // backward
                            : cv::Scalar(0, 255, 0); // forward

        cv::arrowedLine(
            img,
            cv::Point(x0, y0),
            cv::Point(x1, y1),
            color,
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

        int idx = 0;

        while (reader.isOpen()) {
            FrameData res = reader.read();
            if (!res.ok)
                break;

            std::cout << "Frame " << idx++
                      << " type=" << res.pict_type
                      << " mv=" << res.motion_vectors.size()
                      << std::endl;

            cv::Mat img(
                res.image.height,
                res.image.width,
                CV_8UC3,
                res.image.data,
                res.image.stride
            );

            draw_motion_vectors(img, res.motion_vectors, 1, 1.0f);

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
