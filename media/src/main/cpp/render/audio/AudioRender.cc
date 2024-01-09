//
// Created by 郭磊 on 2024/1/8.
//

#include "AudioRender.h"


AudioRender::AudioRender() {
  pthread_mutex_init(&m_Mutex, NULL);
  pthread_cond_init(&m_Cond, NULL);
}


void AudioRender::Init() {
  LOGD("AudioRender::Init");
  int result = -1;
  do {
    // 创建播放声音的引擎
    result = CreateEngine();
    if (result != SL_RESULT_SUCCESS) {
      LOGD("OpenSLRender::Init CreateEngine fail. result=%d", result);
      break;
    }
    // 创建混音器
    result = CreateOutputMixer();
    if (result != SL_RESULT_SUCCESS) {
      LOGD("OpenSLRender::Init CreateOutputMixer fail. result=%d", result);
      break;
    }
    // 创建播放器
    result = CreateAudioPlayer();
    if (result != SL_RESULT_SUCCESS) {
      LOGD("OpenSLRender::Init CreateAudioPlayer fail. result=%d", result);
      break;
    }
    LOGD("OpenSLRender::Init OpenSL ES success");
    detach_thread_create(NULL, (void *) StartRenderThread, this);
  } while (false);
  if (result != SL_RESULT_SUCCESS) {
    LOGD("OpenSLRender::Init fail. result=%d", result);
    UnInit();
  }
}

int AudioRender::CreateEngine() {
  SLresult result = SL_RESULT_SUCCESS;
  do {
    result = slCreateEngine(&m_EngineObject, 0, NULL, 0, NULL, NULL);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("AudioRender::CreateEngine sl create engine is failed, result:%d", result);
      break;
    }
    // 实现（Realize）engineObject接口对象
    result = (*m_EngineObject)->Realize(m_EngineObject, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("AudioRender::CreateEngine Realize fail, result:%d", result);
      break;
    }
    // 从引擎对象中获取引擎
    result = (*m_EngineObject)->GetInterface(m_EngineObject, SL_IID_ENGINE, &m_EngineEngine);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("AudioRender::CreateEngine GetInterface fail, result:%d", result);
      break;
    }
  } while (false);
  return result;
}

int AudioRender::CreateOutputMixer() {
  SLresult result = SL_RESULT_SUCCESS;
  do {
    // 创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*m_EngineEngine)->CreateOutputMix(m_EngineEngine, &m_OutputMixObj, 1, mids, mreq);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("AudioRender::CreateOutputMixer() CreateOutputMix fail. result=%d", result);
      break;
    }
    result = (*m_OutputMixObj)->Realize(m_OutputMixObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("AudioRender::CreateOutputMixer() Realize fail. result=%d", result);
      break;
    }
  } while (false);
  return result;
}

int AudioRender::CreateAudioPlayer() {
  // 参数设置
  SLDataLocator_AndroidSimpleBufferQueue
      android_queue = {.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, .numBuffers= 2};
  SLDataFormat_PCM pcm = {
      .formatType = SL_DATAFORMAT_PCM,
      .numChannels = (SLuint32) 2,
      .samplesPerSec = SL_SAMPLINGRATE_44_1,
      .bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16,
      .containerSize = SL_PCMSAMPLEFORMAT_FIXED_16,
      .channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
      .endianness = SL_BYTEORDER_LITTLEENDIAN
  };
  SLDataSource slDataSource = {
      .pLocator = &android_queue,
      .pFormat = &pcm
  };
  SLDataLocator_OutputMix outputMix = {
      .locatorType = SL_DATALOCATOR_OUTPUTMIX,
      .outputMix = m_OutputMixObj
  };
  SLDataSink slDataSink = {.pLocator= &outputMix, .pFormat = nullptr};
  const SLInterfaceID ids[3] = {SL_IID_BUFFERQUEUE, SL_IID_EFFECTSEND, SL_IID_VOLUME};
  const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

  SLresult result = SL_RESULT_SUCCESS;

  do {
    // 创建播放器
    result = (*m_EngineEngine)->CreateAudioPlayer(m_EngineEngine,
                                                  &m_AudioPlayerObj,
                                                  &slDataSource,
                                                  &slDataSink,
                                                  3,
                                                  ids,
                                                  req);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("AudioRender::CreateAudioPlayer Create Audio Player is failed. result=%d", result);
      break;
    }
    result = (*m_AudioPlayerObj)->Realize(m_AudioPlayerObj, SL_BOOLEAN_FALSE);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("AudioRender::CreateAudioPlayer Realize is failed. result=%d", result);
      break;
    }
    // 获取player对象
    result = (*m_AudioPlayerObj)->GetInterface(m_AudioPlayerObj, SL_IID_PLAY, &m_AudioPlayerPlay);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("AudioRender::CreateAudioPlayer GetInterface m_AudioPlayerPlay is failed. result=%d",
           result);
      break;
    }
    // 获取数据填充的buffer
    result =
        (*m_AudioPlayerObj)->GetInterface(m_AudioPlayerObj, SL_IID_BUFFERQUEUE, &m_BufferQueue);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("AudioRender::CreateAudioPlayer GetInterface m_BufferQueue is failed. result=%d",
           result);
      break;
    }
    result = (*m_BufferQueue)->RegisterCallback(m_BufferQueue, AudioPlayerCallback, this);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("AudioRender::CreateAudioPlayer RegisterCallback is failed. result=%d",
           result);
      break;
    }
    // 获取声音控制器
    result =
        (*m_AudioPlayerObj)->GetInterface(m_AudioPlayerObj, SL_IID_VOLUME, &m_AudioPlayerVolume);
    if (result != SL_RESULT_SUCCESS) {
      LOGD("OpenSLRender::CreateAudioPlayer GetInterface m_AudioPlayerVolume fail. result=%d",
           result);
      break;
    }
  } while (false);
  return result;
}

void AudioRender::StartRenderThread(void *args) {
  AudioRender *render = static_cast<AudioRender *>(args);
  if (render != NULL) {
    render->RunRender();
  }
}

void AudioRender::RunRender() {
  // 这里应该检查一下播放的队列是否有数据，没有数据应该进行等待，知道有数据
  while (GetAudioFrameQueueSize() < MAX_QUEUE_BUFFER_SIZE) {
    // 这里进行等待
    pthread_mutex_lock(&m_Mutex);
    // TODO 直接睡行不行？ 多线程操作 如果是单线程操作不需要挂锁吧。。。。
    usleep(1000 * 10);
    LOGD("AudioRender::RunRender sleep until queue size is ok");
    pthread_mutex_unlock(&m_Mutex);
  }
  // 为了播放，首次必须调用一下设置的回调函数
  (*m_AudioPlayerPlay)->SetPlayState(m_AudioPlayerPlay, SL_PLAYSTATE_PLAYING);
  AudioPlayerCallback(m_BufferQueue, this);
}

void AudioRender::AudioPlayerCallback(SLAndroidSimpleBufferQueueItf bufferQueue, void *context) {
  // 通过这个回调函数进行喂数据
  AudioRender *render = static_cast<AudioRender *>(context);
  if (render != NULL) {
    render->HandleAudioFrameQueue();
  }
}

int AudioRender::GetAudioFrameQueueSize() {
  return queue.size();
}

void AudioRender::HandleAudioFrameQueue() {
  // 将数据喂给buffer
  while (GetAudioFrameQueueSize() < MAX_QUEUE_BUFFER_SIZE) {
    // 这里进行等待
    pthread_mutex_lock(&m_Mutex);
    // TODO 直接睡行不行？ 多线程操作 如果是单线程操作不需要挂锁吧。。。。
    usleep(1000 * 10);
    LOGD("AudioRender::RunRender sleep until queue size is ok");
    pthread_mutex_unlock(&m_Mutex);
  }
  AudioFrame *audioFrame = queue.front();
  if (audioFrame != NULL) {
    (*m_BufferQueue)->Enqueue(m_BufferQueue, audioFrame->data, audioFrame->dataSize);
    queue.pop();
    delete audioFrame;
  }
}

void AudioRender::RenderAudioFrame(uint8_t *out, int dataSize) {
  // 构建对应的AudioFrame然后将其放入队列
  LOGD("AudioRender::RenderAudioFrame pData=%p, dataSize=%d", out, dataSize);
  if (out != NULL && dataSize > 0) {
    while (GetAudioFrameQueueSize() >= MAX_QUEUE_BUFFER_SIZE) {
      usleep(1000 * 10);
    }
    AudioFrame *audioFrame = new AudioFrame(out, dataSize);
    queue.push(audioFrame);
  }
}

void AudioRender::UnInit() {
  LOGD("OpenSLRender::UnInit");
  // 释放资源

  if (m_AudioPlayerPlay) {
    (*m_AudioPlayerPlay)->SetPlayState(m_AudioPlayerPlay, SL_PLAYSTATE_STOPPED);
    m_AudioPlayerPlay = nullptr;
  }


  if (m_AudioPlayerObj) {
    (*m_AudioPlayerObj)->Destroy(m_AudioPlayerObj);
    m_AudioPlayerObj = nullptr;
    m_BufferQueue = nullptr;
  }

  if (m_OutputMixObj) {
    (*m_OutputMixObj)->Destroy(m_OutputMixObj);
    m_OutputMixObj = nullptr;
  }

  if (m_EngineObject) {
    (*m_EngineObject)->Destroy(m_EngineObject);
    m_EngineObject = nullptr;
    m_EngineEngine = nullptr;
  }

  for (int i = 0; i < queue.size(); ++i) {
    AudioFrame *audioFrame = queue.front();
    queue.pop();
    delete audioFrame;
  }
}

AudioRender::~AudioRender() {
  pthread_mutex_destroy(&m_Mutex);
  pthread_cond_destroy(&m_Cond);
}