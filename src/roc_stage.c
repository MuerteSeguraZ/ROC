#include "roc_stage.h"
#include "roc_workflow.h"
#include "roc_job.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RStage* create_stage(const char* name, int priority) {
    RStage* stage = (RStage*)malloc(sizeof(RStage));
    strcpy(stage->name, name);
    stage->item_count = 0;
    stage->priority = priority;
    stage->status = STAGE_PENDING;
    pthread_mutex_init(&stage->lock, NULL);
    return stage;
}

void destroy_stage(RStage* stage) {
    pthread_mutex_destroy(&stage->lock);
    free(stage);
}

int stage_add_item(RStage* stage, void* item, StageItemType type) {
    pthread_mutex_lock(&stage->lock);
    if (stage->item_count >= MAX_ITEMS_PER_STAGE) {
        pthread_mutex_unlock(&stage->lock);
        return 0;
    }
    stage->items[stage->item_count].item = item;
    stage->items[stage->item_count].type = type;
    stage->item_count++;
    pthread_mutex_unlock(&stage->lock);
    return 1;
}

static void update_stage_status(RStage* stage) {
    pthread_mutex_lock(&stage->lock);
    int all_completed = 1;
    int any_failed = 0;

    for (int i = 0; i < stage->item_count; i++) {
        StageItem* si = &stage->items[i];
        if (si->type == STAGE_ITEM_JOB) {
            JobStatus js = job_status((RJob*)si->item);
            if (js != JOB_COMPLETED) all_completed = 0;
            if (js == JOB_FAILED) any_failed = 1;
        } else if (si->type == STAGE_ITEM_WORKFLOW) {
            WorkflowStatus ws = workflow_status((RWorkflow*)si->item);
            if (ws != WORKFLOW_COMPLETED) all_completed = 0;
            if (ws == WORKFLOW_FAILED) any_failed = 1;
        }
    }

    if (any_failed) stage->status = STAGE_FAILED;
    else if (all_completed) stage->status = STAGE_COMPLETED;
    else stage->status = STAGE_RUNNING;

    pthread_mutex_unlock(&stage->lock);
}

int stage_run(RStage* stage, RTaskScheduler* sched) {
    pthread_mutex_lock(&stage->lock);
    stage->status = STAGE_RUNNING;
    pthread_mutex_unlock(&stage->lock);

    for (int i = 0; i < stage->item_count; i++) {
        StageItem* si = &stage->items[i];
        if (si->type == STAGE_ITEM_JOB) job_run((RJob*)si->item, sched);
        else if (si->type == STAGE_ITEM_WORKFLOW) workflow_run((RWorkflow*)si->item, sched);
    }
    return 1;
}


StageStatus stage_status(RStage* stage) {
    update_stage_status(stage);
    pthread_mutex_lock(&stage->lock);
    StageStatus s = stage->status;
    pthread_mutex_unlock(&stage->lock);
    return s;
}
