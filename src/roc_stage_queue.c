#include <windows.h>
#include "roc_stage_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RStageQueue* create_stage_queue(const char* name) {
    RStageQueue* queue = (RStageQueue*)malloc(sizeof(RStageQueue));
    strcpy(queue->name, name);
    queue->stage_count = 0;
    pthread_mutex_init(&queue->lock, NULL);
    return queue;
}

void destroy_stage_queue(RStageQueue* queue) {
    pthread_mutex_destroy(&queue->lock);
    free(queue);
}

int stage_queue_add(RStageQueue* queue, RStage* stage) {
    pthread_mutex_lock(&queue->lock);
    if (queue->stage_count >= MAX_STAGES_PER_QUEUE) {
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }
    queue->stages[queue->stage_count++] = stage;
    pthread_mutex_unlock(&queue->lock);
    return 1;
}

// Run stages sequentially
int stage_queue_run(RStageQueue* queue, RTaskScheduler* sched) {
    pthread_mutex_lock(&queue->lock);
    int count = queue->stage_count;
    pthread_mutex_unlock(&queue->lock);

    for (int i = 0; i < count; i++) {
        printf("[StageQueue] Starting stage '%s'\n", queue->stages[i]->name);
        stage_run(queue->stages[i], sched);

        // Wait until stage completes
        while (stage_status(queue->stages[i]) != STAGE_COMPLETED) {
            Sleep(50);
        }
        printf("[StageQueue] Stage '%s' completed\n", queue->stages[i]->name);
    }
    return 1;
}

// Run stages in priority order (higher priority first)
int stage_queue_run_priority(RStageQueue* queue, RTaskScheduler* sched) {
    pthread_mutex_lock(&queue->lock);
    int count = queue->stage_count;

    // Simple bubble sort by stage priority descending
    for (int i = 0; i < count-1; i++) {
        for (int j = 0; j < count-i-1; j++) {
            if (queue->stages[j]->priority < queue->stages[j+1]->priority) {
                RStage* tmp = queue->stages[j];
                queue->stages[j] = queue->stages[j+1];
                queue->stages[j+1] = tmp;
            }
        }
    }
    pthread_mutex_unlock(&queue->lock);

    return stage_queue_run(queue, sched);
}

// Determine overall queue status
StageStatus stage_queue_status(RStageQueue* queue) {
    pthread_mutex_lock(&queue->lock);
    int all_completed = 1;
    int any_failed = 0;
    for (int i = 0; i < queue->stage_count; i++) {
        StageStatus s = stage_status(queue->stages[i]);
        if (s != STAGE_COMPLETED) all_completed = 0;
        if (s == STAGE_FAILED) any_failed = 1;
    }
    pthread_mutex_unlock(&queue->lock);

    if (any_failed) return STAGE_FAILED;
    else if (all_completed) return STAGE_COMPLETED;
    else return STAGE_RUNNING;
}
