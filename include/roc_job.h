#ifndef ROC_JOB_H
#define ROC_JOB_H

#include "roc_task.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_TASKS_PER_JOB 16

typedef enum {
    JOB_PENDING,
    JOB_RUNNING,
    JOB_COMPLETED,
    JOB_FAILED
} JobStatus;

typedef struct {
    char name[50];
    RTask* tasks[MAX_TASKS_PER_JOB];
    int task_count;

    int priority;          // job-level priority
    JobStatus status;

    pthread_mutex_t lock;

    // Dependency tracking
    int dep_matrix[MAX_TASKS_PER_JOB][MAX_TASKS_PER_JOB]; 
    // dep_matrix[i][j] = 1 means task i depends on task j
} RJob;

// Job operations
RJob* create_job(const char* name, int priority);
void destroy_job(RJob* job);

int job_add_task(RJob* job, RTask* task);
int job_run(RJob* job, RTaskScheduler* sched);  // enqueue all tasks
JobStatus job_status(RJob* job);

#endif
