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
};

#include "log/LogUtil.h"
#include "utils/CThread.h"
#include <unistd.h>

#define MAX_PATH 2048

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
