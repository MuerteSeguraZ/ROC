#ifndef ROC_PIPE_QUEUE_H
#define ROC_PIPE_QUEUE_H

#include "roc_pipe.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_PIPES_PER_QUEUE 16

typedef enum {
    PIPEQUEUE_PENDING,
    PIPEQUEUE_RUNNING,
    PIPEQUEUE_COMPLETED,
    PIPEQUEUE_FAILED
} PipeQueueStatus;

typedef struct {
    char name[50];
    RPipe* pipes[MAX_PIPES_PER_QUEUE];
    int pipe_count;
    PipeQueueStatus status;
    pthread_mutex_t lock;
} RPipeQueue;

// Pipe Queue operations
RPipeQueue* create_pipe_queue(const char* name);
void destroy_pipe_queue(RPipeQueue* queue);

int pipe_queue_add(RPipeQueue* queue, RPipe* pipe);
int pipe_queue_run(RPipeQueue* queue, RTaskScheduler* sched);
PipeQueueStatus pipe_queue_status(RPipeQueue* queue);

#endif
