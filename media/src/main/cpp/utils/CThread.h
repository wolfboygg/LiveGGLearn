//
// Created by 郭磊 on 2023/12/14.
//

#ifndef CSTREAMSERVER_CTHREAD_H
#define CSTREAMSERVER_CTHREAD_H

#include <pthread.h>

int detach_thread_create(pthread_t *thread, void *start_routine, void *args);

#endif //CSTREAMSERVER_CTHREAD_H
