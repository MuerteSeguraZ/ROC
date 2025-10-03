#include <windows.h>
#include "roc_bundle_queue.h"
#include <stdlib.h>
#include <stdio.h>

RBundleQueue* create_bundle_queue() {
    RBundleQueue* queue = (RBundleQueue*)malloc(sizeof(RBundleQueue));
    queue->bundle_count = 0;
    pthread_mutex_init(&queue->lock, NULL);
    return queue;
}

void destroy_bundle_queue(RBundleQueue* queue) {
    pthread_mutex_destroy(&queue->lock);
    free(queue);
}

int enqueue_bundle(RBundleQueue* queue, RBundle* bundle) {
    pthread_mutex_lock(&queue->lock);
    if (queue->bundle_count >= MAX_BUNDLES) {
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }

    // Insert in priority order (higher priority first)
    int i = queue->bundle_count - 1;
    while (i >= 0 && queue->bundles[i]->priority < bundle->priority) {
        queue->bundles[i + 1] = queue->bundles[i];
        i--;
    }
    queue->bundles[i + 1] = bundle;
    queue->bundle_count++;
    pthread_mutex_unlock(&queue->lock);
    return 1;
}

RBundle* dequeue_bundle(RBundleQueue* queue) {
    pthread_mutex_lock(&queue->lock);
    if (queue->bundle_count == 0) {
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }
    RBundle* bundle = queue->bundles[0];
    for (int i = 1; i < queue->bundle_count; i++) {
        queue->bundles[i - 1] = queue->bundles[i];
    }
    queue->bundle_count--;
    pthread_mutex_unlock(&queue->lock);
    return bundle;
}

int process_bundle_queue(RBundleQueue* queue, RTaskScheduler* sched) {
    while (1) {
        RBundle* bundle = dequeue_bundle(queue);
        if (!bundle) break;
        bundle_run(bundle, sched);

        // Wait for bundle to complete
        while (bundle_status(bundle) != BUNDLE_COMPLETED &&
               bundle_status(bundle) != BUNDLE_FAILED) {
            Sleep(50);
        }
        printf("[BundleQueue] Bundle '%s' finished with status %d\n",
               bundle->name, bundle_status(bundle));
    }
    return 1;
}
