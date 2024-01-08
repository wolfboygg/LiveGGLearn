//
// Created by 郭磊 on 2024/1/4.
//

#ifndef LIVEGGLEARN_NATIVERENDER_H
#define LIVEGGLEARN_NATIVERENDER_H

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "../../log/LogUtil.h"
#include "../../common_header_def.h"

class NativeRender {
 public:
  NativeRender(JNIEnv *env, jobject surface);
  virtual ~NativeRender();
  virtual void Init(int videoWidth, int videoHeight, int *dstSize);
  virtual void RenderVideoFrame(NativeImage *pImage);
  virtual void UnInit();
 private:
  ANativeWindow *m_NativeWindow = NULL;
  ANativeWindow_Buffer m_ANativeWindow_Buffer;
  int m_DstWidth = 0;
  int m_DstHeight = 0;

  pthread_mutex_t  m_Mute;

};


#endif //LIVEGGLEARN_NATIVERENDER_H
