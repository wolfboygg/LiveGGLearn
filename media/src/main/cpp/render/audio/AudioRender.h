//
// Created by 郭磊 on 2024/1/8.
//

#ifndef LIVEGGLEARN_AUDIORENDER_H
#define LIVEGGLEARN_AUDIORENDER_H

#include "../../common_header_def.h"

#define MAX_QUEUE_BUFFER_SIZE 3

using namespace std;

// 创建一个需要保存的数据类型
class AudioFrame {
 public:
  AudioFrame(uint8_t *data, int dataSize, bool hardCopy = true) {
    this->dataSize = dataSize;
    this->hardCopy = hardCopy;
    this->data = data;
    // 如果是复制的需要动态开辟内存空间进行数据的的拷贝
    if (hardCopy) {
      this->data = (uint8_t *) malloc(this->dataSize);
      memcpy(this->data, data, this->dataSize);
    }
  }
  ~AudioFrame() {
    if (hardCopy && this->data)
      free(this->data);
    this->data = nullptr;
  }

  uint8_t *data = nullptr;
  int dataSize = 0;
  bool hardCopy = true;
};


/**
 * 使用OpenSLES进行播放
 */

class AudioRender {
 public:
  AudioRender();

  ~AudioRender();

  void Init();

  void UnInit();

  static void AudioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context);

  static void StartRenderThread(void *args);

  void RunRender();

  int GetAudioFrameQueueSize();

  void RenderAudioFrame(uint8_t *out, int dataSize);

 private:
  int CreateEngine();

  int CreateOutputMixer();

  int CreateAudioPlayer();

  void HandleAudioFrameQueue();


 private:
  SLObjectItf m_EngineObject = NULL;
  SLEngineItf m_EngineEngine = NULL;
  SLObjectItf m_OutputMixObj = NULL;
  SLObjectItf m_AudioPlayerObj = NULL;
  SLPlayItf m_AudioPlayerPlay = NULL;
  SLVolumeItf m_AudioPlayerVolume = NULL;
  SLAndroidSimpleBufferQueueItf m_BufferQueue;

  // 播放队列
  queue<AudioFrame *> queue;
  pthread_mutex_t m_Mutex;
  pthread_cond_t m_Cond;

};


#endif //LIVEGGLEARN_AUDIORENDER_H
