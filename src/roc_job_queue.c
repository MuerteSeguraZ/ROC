#include "roc_job_queue.h"
#include <stdlib.h>
#include <string.h>

RJobQueue* create_job_queue() {
    RJobQueue* jq = (RJobQueue*)malloc(sizeof(RJobQueue));
    jq->count = 0;
    pthread_mutex_init(&jq->lock, NULL);
    pthread_cond_init(&jq->cond, NULL);
    return jq;
}

void destroy_job_queue(RJobQueue* jq) {
    pthread_mutex_destroy(&jq->lock);
    pthread_cond_destroy(&jq->cond);
    free(jq);
}

int job_queue_add(RJobQueue* jq, RJob* job) {
    pthread_mutex_lock(&jq->lock);
    if (jq->count >= MAX_JOBS) {
        pthread_mutex_unlock(&jq->lock);
        return 0;
    }
    jq->jobs[jq->count++] = job;
    pthread_cond_signal(&jq->cond);
    pthread_mutex_unlock(&jq->lock);
    return 1;
}

RJob* job_queue_next(RJobQueue* jq) {
    pthread_mutex_lock(&jq->lock);
    int idx = -1;
    int max_priority = -1;
    for (int i = 0; i < jq->count; i++) {
        if (jq->jobs[i]->status == JOB_PENDING && jq->jobs[i]->priority > max_priority) {
            max_priority = jq->jobs[i]->priority;
            idx = i;
        }
    }
    if (idx == -1) {
        pthread_mutex_unlock(&jq->lock);
        return NULL;
    }

    RJob* job = jq->jobs[idx];
    // Shift remaining jobs down
    for (int i = idx; i < jq->count - 1; i++)
        jq->jobs[i] = jq->jobs[i + 1];
    jq->count--;

    pthread_mutex_unlock(&jq->lock);
    return job;
}
