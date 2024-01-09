//
// Created by 郭磊 on 2024/1/8.
//

#ifndef LIVEGGLEARN_AUDIODECODER_H
#define LIVEGGLEARN_AUDIODECODER_H

#include "../../common_header_def.h"
#include "../../render/audio/AudioRender.h"

/**
 * 音频解码器 这里先暂时这样写，完事之后在整合成继承的方式进行处理
 */
class AudioDecoder {

 public:
  AudioDecoder();

  ~AudioDecoder();

  void Init(const char *video_path);

  void CleanResource();

  void OnDecoderReady();

 public:
  static void StartAudioDecoder(void *);

  void RunAudioDecoder();

  void OnFrameAvailable(AVFrame *avFrame);

  void SetAudioRender(AudioRender *render);

  double GetAudioClock();

 private:
  char resource_url[MAX_PATH] = {0};

  AVFormatContext *m_AVFormatContext = NULL;
  AVCodecParameters *m_AVCodecParameters = NULL;
  const AVCodec *m_AVCodec = NULL;
  AVCodecContext *m_AVCodecContext = NULL;

  AVPacket *m_AVPacket = NULL;
  AVFrame *m_AVFrame = NULL;

  int m_StreamIndex = -1;

  AudioRender *m_AudioRender = NULL;

  // 声音用来做重采样处理
  SwrContext *m_SwrContext = NULL;

  //number of sample per channel
  int m_nbSamples = 0;

  //dst frame data size
  int m_DstFrameDataSze = 0;

  uint8_t *m_AudioOutBuffer = nullptr;

  double m_Clock = 0.0;

  AVRational m_StreamTimeBase;

};


#endif //LIVEGGLEARN_AUDIODECODER_H
