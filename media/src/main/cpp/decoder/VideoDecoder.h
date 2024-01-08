//
// Created by 郭磊 on 2024/1/3.
//

#ifndef LIVEGGLEARN_VIDEODECODER_H
#define LIVEGGLEARN_VIDEODECODER_H

#include "../common_header_def.h"
#include "../render/video/NativeRender.h"

class VideoDecoder {
 public:
  VideoDecoder();

  ~VideoDecoder();

  void Init(const char *url);

  void OnDecoderReady();

  void CleanResource();

  void SetVideoRender(NativeRender *render);

  void OnFrameAvailable(AVFrame *avFrame);

  int GetRenderWidth();

  int GetRenderHeight();

 public:
  static void StartVideoDecoder(void *);

  void RunVideoDecoder();

 private:
  char resource_url[MAX_PATH] = {0};
  AVFormatContext *m_AVFormatContext = NULL;
  const AVCodec *m_AVCodec = NULL;
  AVCodecContext *m_AVCodecContext = NULL;
  AVPacket *m_AVPack = NULL;
  AVFrame *m_AVFrame = NULL;
  // 流索引
  int m_StreamIndex = -1;
  double m_fps = 0.0;

  NativeRender *m_Render = NULL;

  int m_VideoWidth = 0;
  int m_VideoHeight= 0;

  int m_RenderWidth = 0;
  int m_RenderHeight = 0;

  AVFrame *m_RGBAFrame = NULL;
  const AVPixelFormat DST_PIXEL_FORMAT = AV_PIX_FMT_RGBA;

  uint8_t  *m_FrameBuffer = NULL;
  SwsContext *m_SwsContext = NULL;

};


#endif //LIVEGGLEARN_VIDEODECODER_H
