//
// Created by 郭磊 on 2024/1/3.
//

#ifndef LIVEGGLEARN_COMMON_HEADER_DEF_H
#define LIVEGGLEARN_COMMON_HEADER_DEF_H
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/time.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
};

#include "log/LogUtil.h"
#include "utils/CThread.h"
#include <unistd.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <queue>

#define MAX_PATH 2048

// 音频编码采样率
static const int AUDIO_DST_SAMPLE_RATE = 44100;
// 音频编码通道数
static const int AUDIO_DST_CHANNEL_COUNTS = 2;
// 音频编码声道格式
static const uint64_t AUDIO_DST_CHANNEL_LAYOUT = AV_CH_LAYOUT_STEREO;
// 音频编码比特率
static const int AUDIO_DST_BIT_RATE = 64000;
// ACC音频一帧采样数
static const int ACC_NB_SAMPLES = 1024;

static const AVSampleFormat DST_SAMPLT_FORMAT = AV_SAMPLE_FMT_S16;

#define AV_SYNC_THRESHOLD_MIN 0.04
#define AV_SYNC_THRESHOLD_MAX 0.1

/**
 * 本地解码完的frame保存的数据结构
 */
typedef struct tag_NativeImage {
  int width;
  int height;
  int format;
  uint8_t *ppPlane[3];
  int pLineSize[3];

  tag_NativeImage() {
    width = 0;
    height = 0;
    format = 0;
    ppPlane[0] = nullptr;
    ppPlane[1] = nullptr;
    ppPlane[2] = nullptr;
    pLineSize[0] = 0;
    pLineSize[1] = 0;
    pLineSize[2] = 0;
  }
} NativeImage;


#endif //LIVEGGLEARN_COMMON_HEADER_DEF_H
