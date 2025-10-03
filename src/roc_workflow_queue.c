#include "roc_workflow_queue.h"
#include <stdlib.h>
#include <stdio.h>

RWorkflowQueue* create_workflow_queue() {
    RWorkflowQueue* q = (RWorkflowQueue*)malloc(sizeof(RWorkflowQueue));
    q->count = 0;
    pthread_mutex_init(&q->lock, NULL);
    return q;
}

void destroy_workflow_queue(RWorkflowQueue* queue) {
    pthread_mutex_destroy(&queue->lock);
    free(queue);
}

// Insert workflow in priority order (higher priority first)
int workflow_queue_add(RWorkflowQueue* queue, RWorkflow* wf) {
    pthread_mutex_lock(&queue->lock);
    if (queue->count >= MAX_WORKFLOWS) {
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }

    // Insert at correct position based on priority
    int i = queue->count - 1;
    while (i >= 0 && queue->workflows[i]->priority < wf->priority) {
        queue->workflows[i + 1] = queue->workflows[i];
        i--;
    }
    queue->workflows[i + 1] = wf;
    queue->count++;
    pthread_mutex_unlock(&queue->lock);
    return 1;
}

// Run all workflows in queue in parallel
int workflow_queue_run(RWorkflowQueue* queue, RTaskScheduler* sched) {
    pthread_mutex_lock(&queue->lock);
    for (int i = 0; i < queue->count; i++) {
        workflow_run(queue->workflows[i], sched);
    }
    pthread_mutex_unlock(&queue->lock);
    return 1;
}

// Check if all workflows have completed
int workflow_queue_all_completed(RWorkflowQueue* queue) {
    int all_done = 1;
    pthread_mutex_lock(&queue->lock);
    for (int i = 0; i < queue->count; i++) {
        if (workflow_status(queue->workflows[i]) != WORKFLOW_COMPLETED)
            all_done = 0;
    }
    pthread_mutex_unlock(&queue->lock);
    return all_done;
}
