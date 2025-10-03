#ifndef ROC_WORKFLOW_H
#define ROC_WORKFLOW_H

#include "roc_task.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_TASKS_PER_WORKFLOW 32

typedef enum {
    WORKFLOW_PENDING,
    WORKFLOW_RUNNING,
    WORKFLOW_COMPLETED,
    WORKFLOW_FAILED
} WorkflowStatus;

typedef struct {
    char name[50];
    RTask* tasks[MAX_TASKS_PER_WORKFLOW];
    int task_count;

    int priority;           // Workflow-level priority
    WorkflowStatus status;

    pthread_mutex_t lock;
} RWorkflow;

// Workflow operations
RWorkflow* create_workflow(const char* name, int priority);
void destroy_workflow(RWorkflow* wf);

int workflow_add_task(RWorkflow* wf, RTask* task);
int workflow_run(RWorkflow* wf, RTaskScheduler* sched);  // enqueue all tasks
WorkflowStatus workflow_status(RWorkflow* wf);

#endif
