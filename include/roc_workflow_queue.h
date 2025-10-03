#ifndef ROC_WORKFLOW_QUEUE_H
#define ROC_WORKFLOW_QUEUE_H

#include "roc_workflow.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_WORKFLOWS 32

typedef struct {
    RWorkflow* workflows[MAX_WORKFLOWS];
    int count;
    pthread_mutex_t lock;
} RWorkflowQueue;

// Queue operations
RWorkflowQueue* create_workflow_queue();
void destroy_workflow_queue(RWorkflowQueue* queue);

int workflow_queue_add(RWorkflowQueue* queue, RWorkflow* wf);
int workflow_queue_run(RWorkflowQueue* queue, RTaskScheduler* sched); // runs workflows based on priority
int workflow_queue_all_completed(RWorkflowQueue* queue);

#endif
