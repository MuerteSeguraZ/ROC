#ifndef ROC_STAGE_QUEUE_H
#define ROC_STAGE_QUEUE_H

#include "roc_stage.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_STAGES_PER_QUEUE 16

typedef struct {
    char name[50];
    RStage* stages[MAX_STAGES_PER_QUEUE];
    int stage_count;
    pthread_mutex_t lock;
} RStageQueue;

// Stage Queue operations
RStageQueue* create_stage_queue(const char* name);
void destroy_stage_queue(RStageQueue* queue);

int stage_queue_add(RStageQueue* queue, RStage* stage);
int stage_queue_run(RStageQueue* queue, RTaskScheduler* sched);   // Run all stages sequentially
int stage_queue_run_priority(RStageQueue* queue, RTaskScheduler* sched); // Run stages by priority
StageStatus stage_queue_status(RStageQueue* queue);

#endif
