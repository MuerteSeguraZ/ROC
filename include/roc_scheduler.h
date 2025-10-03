#ifndef ROC_SCHEDULER_H
#define ROC_SCHEDULER_H

#include "roc_task.h"
#include "roc.h"
#include <pthread.h>

#define MAX_TASK_QUEUE 128

typedef struct {
    RTask* queue[MAX_TASK_QUEUE];
    int count;

    pthread_mutex_t lock;
    pthread_cond_t cond;

    int running; // 1 = scheduler active, 0 = stop
    pthread_t thread;
} RTaskScheduler;

// =====================
// Scheduler operations
// =====================
RTaskScheduler* create_scheduler();
void destroy_scheduler(RTaskScheduler* sched);

// Add a task to the queue
int scheduler_add_task(RTaskScheduler* sched, RTask* task);

// Start the scheduler thread
void scheduler_start(RTaskScheduler* sched);

// Stop the scheduler thread
void scheduler_stop(RTaskScheduler* sched);

#endif
