#include "roc_thread.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// Internal wrapper to update running state
static void* thread_wrapper(void* arg) {
    RThread* t = (RThread*)arg;
    if (t->func) {
        t->func(t->arg);
    }
    t->running = false;
    return NULL;
}

// Create a thread object
RThread* thread_create(void (*func)(void*), void* arg) {
    RThread* t = (RThread*)malloc(sizeof(RThread));
    if (!t) return NULL;
    t->func = func;
    t->arg = arg;
    t->running = false;
    return t;
}

// Start the thread
bool thread_start(RThread* t) {
    if (!t) return false;
    if (t->running) return false; // Already running

    int res = pthread_create(&t->tid, NULL, thread_wrapper, t);
    if (res != 0) {
        perror("pthread_create failed");
        return false;
    }
    t->running = true;
    return true;
}

// Wait for thread to finish
void thread_join(RThread* t) {
    if (!t || !t->running) return;
    pthread_join(t->tid, NULL);
    t->running = false;
}

// Stop thread (cooperative)
void thread_stop(RThread* t) {
    if (!t) return;
    t->running = false;  // Your func should check this flag periodically
}
