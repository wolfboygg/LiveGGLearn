cmake_minimum_required(VERSION 3.18.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")

set(jnilibs ${CMAKE_SOURCE_DIR}/../jniLibs)

project("media")

link_directories(
        ${jnilibs}/${ANDROID_ABI})

include_directories(${CMAKE_SOURCE_DIR}/ffmpeg/include)

add_library(
        media
        SHARED
        native_learn.cc
        decoder/VideoDecoder.cc
        utils/CThread.cpp
        render/video/NativeRender.cc
)


find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

add_library(avutil
        SHARED
        IMPORTED)
set_target_properties(avutil
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs}/${ANDROID_ABI}/libavutil.so)

add_library(swresample
        SHARED
        IMPORTED)
set_target_properties(swresample
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs}/${ANDROID_ABI}/libswresample.so)

add_library(avcodec
        SHARED
        IMPORTED)
set_target_properties(avcodec
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs}/${ANDROID_ABI}/libavcodec.so)

add_library(avfilter
        SHARED
        IMPORTED)
set_target_properties(avfilter
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs}/${ANDROID_ABI}/libavfilter.so)

add_library(swscale
        SHARED
        IMPORTED)
set_target_properties(swscale
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs}/${ANDROID_ABI}/libswscale.so)

add_library(avformat
        SHARED
        IMPORTED)
set_target_properties(avformat
        PROPERTIES IMPORTED_LOCATION
        ${jnilibs}/${ANDROID_ABI}/libavformat.so)

set(native-libs
        android
        mediandk
        EGL
        GLESv3
        OpenSLES
        log
        m
        z
        )


target_link_libraries(
        media
        avcodec avfilter avformat avutil swresample swscale
        ${log-lib}
        ${native-libs}
)