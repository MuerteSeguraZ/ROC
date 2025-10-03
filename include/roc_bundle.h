#ifndef ROC_BUNDLE_H
#define ROC_BUNDLE_H

#include "roc_job.h"
#include "roc_workflow.h"
#include "roc_task.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_ITEMS_PER_BUNDLE 32

typedef enum {
    BUNDLE_PENDING,
    BUNDLE_RUNNING,
    BUNDLE_COMPLETED,
    BUNDLE_FAILED
} BundleStatus;

typedef enum {
    BUNDLE_ITEM_JOB,
    BUNDLE_ITEM_WORKFLOW,
    BUNDLE_ITEM_TASK
} BundleItemType;

typedef struct {
    void* item;
    BundleItemType type;
} BundleItem;

typedef struct {
    char name[50];
    BundleItem items[MAX_ITEMS_PER_BUNDLE];
    int item_count;

    int priority;         // bundle-level priority
    BundleStatus status;

    pthread_mutex_t lock;
} RBundle;

// Bundle operations
RBundle* create_bundle(const char* name, int priority);
void destroy_bundle(RBundle* bundle);

int bundle_add_job(RBundle* bundle, RJob* job);
int bundle_add_workflow(RBundle* bundle, RWorkflow* wf);
int bundle_add_task(RBundle* bundle, RTask* task);

int bundle_run(RBundle* bundle, RTaskScheduler* sched);
BundleStatus bundle_status(RBundle* bundle);

#endif
