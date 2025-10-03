#ifndef ROC_TASK_H
#define ROC_TASK_H

#include "roc.h"
#include <pthread.h>

#define MAX_RESOURCES_PER_TASK 8

typedef enum {
    TASK_PENDING,
    TASK_RUNNING,
    TASK_COMPLETED,
    TASK_FAILED
} TaskStatus;

// Represents a single resource requirement
typedef struct {
    RNode* node;   // Optional: preselected node, NULL = any
    int amount;    // Units required
} TaskResourceReq;

// Represents a task/job in ROC
typedef struct {
    char name[50];
    TaskResourceReq resources[MAX_RESOURCES_PER_TASK];
    int resource_count;

    int priority;        // Higher = more urgent
    TaskStatus status;

    pthread_mutex_t lock;
} RTask;

// =====================
// Task operations
// =====================
RTask* create_task(const char* name, int priority);
void destroy_task(RTask* task);

int add_resource_req(RTask* task, RNode* node, int amount);
int remove_resource_req(RTask* task, int index);

int allocate_task(RTask* task);     // Reserve all resources
void release_task(RTask* task);     // Release all resources

int run_task(RTask* task);           // Simulate execution based on allocated resources

TaskStatus task_status(RTask* task);
int run_task_async(RTask* task);

#endif
