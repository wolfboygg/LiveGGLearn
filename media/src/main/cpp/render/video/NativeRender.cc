//
// Created by 郭磊 on 2024/1/4.
//

#include "NativeRender.h"

NativeRender::NativeRender(JNIEnv *env, jobject surface) {
  m_NativeWindow = ANativeWindow_fromSurface(env, surface);
  pthread_mutex_init(&m_Mute, NULL);
}

void NativeRender::Init(int videoWidth, int videoHeight, int *dstSize) {
  if (m_NativeWindow == NULL) {
    LOGD("m_nativeWindow init failed\n");
    return;
  }
  // 计算宽高比算出真正的width and height
  int windowWidth = ANativeWindow_getWidth(m_NativeWindow);
  int windowHeight = ANativeWindow_getWidth(m_NativeWindow);
  // 通过传入的videoWidth和videoHeight来以及显示的surface大小来判断真是的宽高
  if (windowWidth < windowHeight * videoWidth / videoHeight) {
    m_DstWidth = windowWidth;
    m_DstHeight = windowWidth * videoHeight / videoWidth;
  } else {
    m_DstWidth = windowHeight * videoWidth / videoHeight;
    m_DstHeight = windowHeight;
  }
  LOGD("NativeRender::Init window[w,h]=[%d, %d],DstSize[w, h]=[%d, %d]",
       windowWidth,
       windowHeight,
       m_DstWidth,
       m_DstHeight);

  *dstSize++ = m_DstWidth;
  *dstSize = m_DstHeight;
  // 给ANativeWindow设置区域大小
  ANativeWindow_setBuffersGeometry(m_NativeWindow,
                                   m_DstWidth,
                                   m_DstHeight,
                                   WINDOW_FORMAT_RGBA_8888);
}

void NativeRender::RenderVideoFrame(NativeImage *pImage) {
  // 进行数据拷贝 这里已经是转换完成的argb
  pthread_mutex_lock(&m_Mute);
  int ret = ANativeWindow_lock(m_NativeWindow, &m_ANativeWindow_Buffer, NULL);
  if (ret != 0) {
    LOGD("RenderVideoFrame is error");
    pthread_mutex_unlock(&m_Mute);
    ANativeWindow_unlockAndPost(m_NativeWindow);
    return;
  }
  // 将数据拷贝到m_ANativeWindow_Buffer 一样一行拷贝
  uint8_t *out = static_cast<uint8_t *>(m_ANativeWindow_Buffer.bits);
  // 设置步长 //这块代码需要详细捋清思路
  int srcLineSize = pImage->width * 4;// 一样有多少像素
  int dstLineSize = m_ANativeWindow_Buffer.stride * 4; // 多少缓冲区
  for (int i = 0; i < m_DstHeight; ++i) {
    memcpy(out + i * dstLineSize, pImage->ppPlane[0] + i * srcLineSize, srcLineSize);
  }
  ANativeWindow_unlockAndPost(m_NativeWindow);
  pthread_mutex_unlock(&m_Mute);
}

void NativeRender::UnInit() {
  pthread_mutex_destroy(&m_Mute);
}

NativeRender::~NativeRender() {
  if (m_NativeWindow != NULL) {
    ANativeWindow_release(m_NativeWindow);
  }
}
