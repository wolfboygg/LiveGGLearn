//
// Created by 郭磊 on 2024/1/3.
//

#include "common_header_def.h"
#include <jni.h>
#include "decoder/VideoDecoder.h"
#include "render/video/NativeRender.h"


extern "C"
JNIEXPORT jintArray JNICALL
Java_com_putong_media_JNIHelper_getFFmpegInfo(JNIEnv *env,
                                              jobject thiz,
                                              jstring path,
                                              jobject surface) {
  const char *video_path = env->GetStringUTFChars(path, 0);
  const char *config_info = avformat_configuration();
  NativeRender *render = new NativeRender(env, surface);
  VideoDecoder *videoDecoder = new VideoDecoder();
  videoDecoder->SetVideoRender(render);
  videoDecoder->Init(video_path);
  env->ReleaseStringUTFChars(path, video_path);
  int render_width = videoDecoder->GetRenderWidth();
  int render_height = videoDecoder->GetRenderHeight();
  jintArray array = env->NewIntArray(2);
  jint *arr = env->GetIntArrayElements(array, NULL);
  LOGD("native_learn:width:%d, height:%d\n", render_width, render_height);
  *(arr + 0) = render_width;
  *(arr + 1) = render_height;
//  env->ReleaseIntArrayElements(array, arr, 0);
  LOGD("native_learn:width:%d, height:%d\n", *(arr + 0), *(arr + 1));
  env->SetIntArrayRegion(array, 0, 2, arr);
  return array;
//  return env->NewStringUTF(config_info);
}
