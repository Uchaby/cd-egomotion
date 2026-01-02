#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include "ffmpeg_reader.hpp"
#include "mv_structs.hpp"

extern "C" {
#include <libavutil/mem.h>
}

namespace py = pybind11;

static py::array bgrimage_to_numpy(BGRImage&& img) {
    if (!img.data || img.width <= 0 || img.height <= 0) {
        return py::none();
    }

    const ssize_t h = img.height;
    const ssize_t w = img.width;

    std::vector<ssize_t> shape   = { h, w, 3 };
    std::vector<ssize_t> strides = { img.stride, 3, 1 };

    uint8_t* ptr = img.data;
    img.data = nullptr;

    py::capsule base(ptr, [](void* p) {
        av_free(p);
    });

    return py::array(
        py::buffer_info(
            ptr,
            sizeof(uint8_t),
            py::format_descriptor<uint8_t>::format(),
            3,
            shape,
            strides
        ),
        base
    );
}

static py::array_t<int32_t>
motion_vectors_to_numpy(const std::vector<MotionVector>& mvs) {
    constexpr ssize_t K = 10;
    const ssize_t N = static_cast<ssize_t>(mvs.size());

    py::array_t<int32_t> arr({N, K});
    auto r = arr.mutable_unchecked<2>();

    for (ssize_t i = 0; i < N; ++i) {
        const auto& mv = mvs[i];
        r(i, 0) = mv.src_x;
        r(i, 1) = mv.src_y;
        r(i, 2) = mv.dst_x;
        r(i, 3) = mv.dst_y;
        r(i, 4) = mv.motion_x;
        r(i, 5) = mv.motion_y;
        r(i, 6) = mv.block_w;
        r(i, 7) = mv.block_h;
        r(i, 8) = mv.motion_scale;
        r(i, 9) = mv.source;
    }

    return arr;
}


PYBIND11_MODULE(_ffmpeg_mv, m) {
    m.doc() = "FFmpeg-based video reader with motion vectors (BGR + MVs)";

    py::class_<FFmpegReader>(m, "VideoReader")
        .def(py::init<const std::string&>(),
             py::arg("path"),
             "Create video reader and open stream")

        .def("isOpen",
             &FFmpegReader::isOpen,
             "Check if stream is still open")

        .def("read", [](FFmpegReader& self) {
            FrameData fd = self.read();

            if (!fd.ok) {
                return py::make_tuple(
                    false,                 
                    '?',                   
                    py::none(),            
                    py::array_t<int32_t>() 
                );
            }

            py::array frame = bgrimage_to_numpy(std::move(fd.image));
            py::array mvs   = motion_vectors_to_numpy(fd.motion_vectors);

            return py::make_tuple(
                true,            
                fd.pict_type,    
                frame,           
                mvs              
            );
        });
}
