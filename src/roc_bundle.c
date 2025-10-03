#include "roc_bundle.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RBundle* create_bundle(const char* name, int priority) {
    RBundle* bundle = (RBundle*)malloc(sizeof(RBundle));
    strcpy(bundle->name, name);
    bundle->item_count = 0;
    bundle->priority = priority;
    bundle->status = BUNDLE_PENDING;
    pthread_mutex_init(&bundle->lock, NULL);
    return bundle;
}

void destroy_bundle(RBundle* bundle) {
    pthread_mutex_destroy(&bundle->lock);
    free(bundle);
}

static int bundle_add_item(RBundle* bundle, void* item, BundleItemType type) {
    pthread_mutex_lock(&bundle->lock);
    if (bundle->item_count >= MAX_ITEMS_PER_BUNDLE) {
        pthread_mutex_unlock(&bundle->lock);
        return 0;
    }
    bundle->items[bundle->item_count].item = item;
    bundle->items[bundle->item_count].type = type;
    bundle->item_count++;
    pthread_mutex_unlock(&bundle->lock);
    return 1;
}

int bundle_add_job(RBundle* bundle, RJob* job) {
    return bundle_add_item(bundle, (void*)job, BUNDLE_ITEM_JOB);
}

int bundle_add_workflow(RBundle* bundle, RWorkflow* wf) {
    return bundle_add_item(bundle, (void*)wf, BUNDLE_ITEM_WORKFLOW);
}

int bundle_add_task(RBundle* bundle, RTask* task) {
    return bundle_add_item(bundle, (void*)task, BUNDLE_ITEM_TASK);
}

// Helper to update bundle status
static void update_bundle_status(RBundle* bundle) {
    pthread_mutex_lock(&bundle->lock);
    int all_completed = 1;
    int any_failed = 0;

    for (int i = 0; i < bundle->item_count; i++) {
        BundleItem* bi = &bundle->items[i];
        BundleStatus s = BUNDLE_PENDING;
        switch (bi->type) {
            case BUNDLE_ITEM_JOB:
                s = job_status((RJob*)bi->item) == JOB_COMPLETED ? BUNDLE_COMPLETED :
                    job_status((RJob*)bi->item) == JOB_FAILED ? BUNDLE_FAILED : BUNDLE_RUNNING;
                break;
            case BUNDLE_ITEM_WORKFLOW:
                s = workflow_status((RWorkflow*)bi->item) == WORKFLOW_COMPLETED ? BUNDLE_COMPLETED :
                    workflow_status((RWorkflow*)bi->item) == WORKFLOW_FAILED ? BUNDLE_FAILED : BUNDLE_RUNNING;
                break;
            case BUNDLE_ITEM_TASK:
                s = task_status((RTask*)bi->item) == TASK_COMPLETED ? BUNDLE_COMPLETED :
                    task_status((RTask*)bi->item) == TASK_FAILED ? BUNDLE_FAILED : BUNDLE_RUNNING;
                break;
        }

        if (s != BUNDLE_COMPLETED) all_completed = 0;
        if (s == BUNDLE_FAILED) any_failed = 1;
    }

    if (any_failed) bundle->status = BUNDLE_FAILED;
    else if (all_completed) bundle->status = BUNDLE_COMPLETED;
    else bundle->status = BUNDLE_RUNNING;

    pthread_mutex_unlock(&bundle->lock);
}

int bundle_run(RBundle* bundle, RTaskScheduler* sched) {
    pthread_mutex_lock(&bundle->lock);
    if (bundle->item_count == 0) {
        pthread_mutex_unlock(&bundle->lock);
        return 0;
    }
    bundle->status = BUNDLE_RUNNING;
    pthread_mutex_unlock(&bundle->lock);

    for (int i = 0; i < bundle->item_count; i++) {
        BundleItem* bi = &bundle->items[i];
        switch (bi->type) {
            case BUNDLE_ITEM_JOB:
                job_run((RJob*)bi->item, sched);
                break;
            case BUNDLE_ITEM_WORKFLOW:
                workflow_run((RWorkflow*)bi->item, sched);
                break;
            case BUNDLE_ITEM_TASK:
                scheduler_add_task(sched, (RTask*)bi->item);
                break;
        }
    }
    return 1;
}

BundleStatus bundle_status(RBundle* bundle) {
    update_bundle_status(bundle);
    pthread_mutex_lock(&bundle->lock);
    BundleStatus s = bundle->status;
    pthread_mutex_unlock(&bundle->lock);
    return s;
}
