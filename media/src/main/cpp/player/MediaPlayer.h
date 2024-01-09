//
// Created by 郭磊 on 2024/1/8.
//

#ifndef LIVEGGLEARN_MEDIAPLAYER_H
#define LIVEGGLEARN_MEDIAPLAYER_H

#include "../decoder/video/VideoDecoder.h"
#include "../decoder/audio/AudioDecoder.h"
#include "../common_header_def.h"

/**
 * 构建一个用来全局管理的播放器
 * MediaPlayer面向java层提供功能
 * 内部通过提供VideoDecoder 用来解码视频流 AudioDecoder用来解码音频流
 * Decoder 持有Render引用，用来将解码出来的数据进行渲染 VideoRender用来渲染视频数据 VideoRender用来渲染音频数据
 */
class MediaPlayer {
 public:
  MediaPlayer(JNIEnv *env, jobject surface);

  ~MediaPlayer();

  void Init(const char *path);

  int GetVideoRenderWidth();

  int GetVideoRenderHeight();

 private:
  VideoDecoder *m_VideoDecoder = NULL;
  NativeRender *m_VideoRender = NULL;

  AudioDecoder *m_AudioDecoder = NULL;
  AudioRender *m_AudioRender = NULL;

  JNIEnv *env;
};


#endif //LIVEGGLEARN_MEDIAPLAYER_H
