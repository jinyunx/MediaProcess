cmake_minimum_required(VERSION 3.12)
project(MediaProcess)

set(CMAKE_CXX_STANDARD 11)

set(FFMPEG_PATH "../ffmpeg-windows-dev")
include_directories("${FFMPEG_PATH}/include")
link_directories("${FFMPEG_PATH}/lib")

add_executable(MediaProcess
        main.cpp
        Demuxer.h Decoder.h)

target_link_libraries(MediaProcess
        avformat
        avcodec
        avfilter
        avutil)
