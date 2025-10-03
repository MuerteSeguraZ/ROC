#ifndef ROC_JOB_QUEUE_H
#define ROC_JOB_QUEUE_H

#include "roc_job.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_JOBS 32

typedef struct {
    RJob* jobs[MAX_JOBS];
    int count;

    pthread_mutex_t lock;
    pthread_cond_t cond;
} RJobQueue;

// Job Queue API
RJobQueue* create_job_queue();
void destroy_job_queue(RJobQueue* jq);

int job_queue_add(RJobQueue* jq, RJob* job);
RJob* job_queue_next(RJobQueue* jq); // Pop highest-priority pending job

#endif
