//
// Created by 郭磊 on 2024/1/8.
//

#include "MediaPlayer.h"

MediaPlayer::MediaPlayer(JNIEnv *env, jobject surface) : env(env) {
  m_VideoDecoder = new VideoDecoder();
  m_VideoRender = new NativeRender(env, surface);

  m_AudioDecoder = new AudioDecoder();
  m_AudioRender = new AudioRender();

  m_VideoDecoder->SetAudioDecoder(m_AudioDecoder);
}

void MediaPlayer::Init(const char *path) {
  m_VideoDecoder->SetVideoRender(m_VideoRender);
  m_VideoDecoder->Init(path);

  m_AudioDecoder->SetAudioRender(m_AudioRender);
  m_AudioDecoder->Init(path);
}

int MediaPlayer::GetVideoRenderWidth() {
  if (m_VideoDecoder != NULL) {
    return m_VideoDecoder->GetRenderWidth();
  }
  return -1;
}


int MediaPlayer::GetVideoRenderHeight() {
  if (m_VideoDecoder != NULL) {
    return m_VideoDecoder->GetRenderHeight();
  }
  return -1;
}

MediaPlayer::~MediaPlayer() {
  if (m_VideoDecoder != NULL) {
    delete m_VideoDecoder;
  }
  if (m_VideoRender != NULL) {
    delete m_VideoDecoder;
  }
}

