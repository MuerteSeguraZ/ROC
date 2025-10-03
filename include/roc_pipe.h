#ifndef ROC_PIPE_H
#define ROC_PIPE_H

#include "roc_task.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_TASKS_PER_PIPE 16

typedef enum {
    PIPE_PENDING,
    PIPE_RUNNING,
    PIPE_COMPLETED,
    PIPE_FAILED
} PipeStatus;

typedef struct {
    char name[50];
    RTask* tasks[MAX_TASKS_PER_PIPE];
    int task_count;
    int priority;         // New: pipe-level priority
    PipeStatus status;
    pthread_mutex_t lock;
} RPipe;

// Pipe operations
RPipe* create_pipe(const char* name, int priority);
void destroy_pipe(RPipe* pipe);

int pipe_add_task(RPipe* pipe, RTask* task);
int pipe_run(RPipe* pipe, RTaskScheduler* sched);
PipeStatus pipe_status(RPipe* pipe);

#endif
