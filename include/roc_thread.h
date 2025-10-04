#ifndef ROC_THREAD_H
#define ROC_THREAD_H

#include <pthread.h>
#include <stdbool.h>

typedef struct {
  pthread_t tid;
  void (*func)(void*);
  void* arg;
  bool running;
} RThread;

RThread* thread_create(void (*func)(void*), void* arg);

bool thread_start(RThread* t);
void thread_join(RThread* t);
void thread_stop(RThread* t);

#endif