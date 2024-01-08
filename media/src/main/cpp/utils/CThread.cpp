//
// Created by 郭磊 on 2023/12/14.
//


#include "CThread.h"
#include "../log/LogUtil.h"


int detach_thread_create(pthread_t *thread, void *start_routine, void *args) {
  pthread_attr_t attr;
  pthread_t thread_t;
  int ret = 0;

  if (thread == NULL) {
    thread = &thread_t;
  }
  // 初始化线程属性
  if (pthread_attr_init(&attr)) {
    LOGD("pthread_attr_init is failed\n");
    return -1;
  }
  // 设置detachstate
  if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) {
    LOGD("pthread_attr_setdetachstate is failed\n");
    pthread_attr_destroy(&attr);
    return -1;
  }
  ret = pthread_create(thread, &attr, (void *(*)(void *)) start_routine, args);
  if (ret < 0) {
    LOGD("pthread_create is failed\n");
    pthread_attr_destroy(&attr);
    return -1;
  }
  // 确保资源释放
  ret = pthread_detach(thread_t);
  return ret;
}

