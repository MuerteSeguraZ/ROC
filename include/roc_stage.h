#ifndef ROC_STAGE_H
#define ROC_STAGE_H

#include "roc_job.h"
#include "roc_workflow.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_ITEMS_PER_STAGE 16

typedef enum {
    STAGE_PENDING,
    STAGE_RUNNING,
    STAGE_COMPLETED,
    STAGE_FAILED
} StageStatus;

typedef enum {
    STAGE_ITEM_JOB,
    STAGE_ITEM_WORKFLOW,
    STAGE_ITEM_TASK
} StageItemType;

typedef struct {
    void* item;           // Pointer to RJob or RWorkflow
    StageItemType type;   // Type of item
} StageItem;

typedef struct {
    char name[50];
    StageItem items[MAX_ITEMS_PER_STAGE];
    int item_count;
    int priority;
    StageStatus status;
    pthread_mutex_t lock;
} RStage;

// Stage operations
RStage* create_stage(const char* name, int priority);
void destroy_stage(RStage* stage);
int stage_add_item(RStage* stage, void* item, StageItemType type);
int stage_run(RStage* stage, RTaskScheduler* sched);
StageStatus stage_status(RStage* stage);

#endif
