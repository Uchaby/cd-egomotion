cmake_minimum_required(VERSION 3.16)
project(ffmpeg_mv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------
# Options
# -----------------------
option(BUILD_PYTHON_BINDINGS "Build pybind11 bindings" ON)

# -----------------------
# Find FFmpeg
# -----------------------
find_package(PkgConfig REQUIRED)

pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC  REQUIRED libavcodec)
pkg_check_modules(AVUTIL   REQUIRED libavutil)
pkg_check_modules(SWSCALE  REQUIRED libswscale)

# -----------------------
# Library
# -----------------------
add_library(ffmpeg_mv SHARED
    src/ffmpeg_reader.cpp
    src/extract_mv.cpp
)

target_include_directories(ffmpeg_mv
    PUBLIC
        include
        ${AVFORMAT_INCLUDE_DIRS}
        ${AVCODEC_INCLUDE_DIRS}
        ${AVUTIL_INCLUDE_DIRS}
)

target_link_libraries(ffmpeg_mv
    PUBLIC
        ${AVFORMAT_LIBRARIES}
        ${AVCODEC_LIBRARIES}
        ${AVUTIL_LIBRARIES}
        ${SWSCALE_LIBRARIES}
)

target_compile_definitions(ffmpeg_mv
    PRIVATE
        __STDC_CONSTANT_MACROS
)

# -----------------------
# Test executable
# -----------------------
add_executable(ffmpeg_mv_demo
    src/main.cpp
)

target_link_libraries(ffmpeg_mv_demo
    PRIVATE ffmpeg_mv
)

# -----------------------
# Python bindings
# -----------------------
if (BUILD_PYTHON_BINDINGS)
    find_package(pybind11 REQUIRED)

    pybind11_add_module(_ffmpeg_mv
        src/bindings.cpp
    )

    target_link_libraries(_ffmpeg_mv
        PRIVATE ffmpeg_mv
    )
endif()
