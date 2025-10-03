#ifndef ROC_BUNDLE_QUEUE_H
#define ROC_BUNDLE_QUEUE_H

#include "roc_bundle.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_BUNDLES 32

typedef struct {
    RBundle* bundles[MAX_BUNDLES];
    int bundle_count;
    pthread_mutex_t lock;
} RBundleQueue;

// Bundle Queue operations
RBundleQueue* create_bundle_queue();
void destroy_bundle_queue(RBundleQueue* queue);

int enqueue_bundle(RBundleQueue* queue, RBundle* bundle);
RBundle* dequeue_bundle(RBundleQueue* queue); // Dequeues highest-priority bundle

int process_bundle_queue(RBundleQueue* queue, RTaskScheduler* sched);

#endif
