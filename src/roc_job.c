#include "roc_job.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RJob* create_job(const char* name, int priority) {
    RJob* job = (RJob*)malloc(sizeof(RJob));
    strcpy(job->name, name);
    job->task_count = 0;
    job->priority = priority;
    job->status = JOB_PENDING;
    pthread_mutex_init(&job->lock, NULL);
    return job;
}

void destroy_job(RJob* job) {
    pthread_mutex_destroy(&job->lock);
    free(job);
}

int job_add_task(RJob* job, RTask* task) {
    pthread_mutex_lock(&job->lock);
    if (job->task_count >= MAX_TASKS_PER_JOB) {
        pthread_mutex_unlock(&job->lock);
        return 0;
    }
    job->tasks[job->task_count++] = task;
    pthread_mutex_unlock(&job->lock);
    return 1;
}

// Helper to update job status
static void update_job_status(RJob* job) {
    pthread_mutex_lock(&job->lock);
    int all_completed = 1;
    int any_failed = 0;

    for (int i = 0; i < job->task_count; i++) {
        TaskStatus ts = task_status(job->tasks[i]);
        if (ts != TASK_COMPLETED) all_completed = 0;
        if (ts == TASK_FAILED) any_failed = 1;
    }

    if (any_failed) job->status = JOB_FAILED;
    else if (all_completed) job->status = JOB_COMPLETED;
    else job->status = JOB_RUNNING;

    pthread_mutex_unlock(&job->lock);
}

int job_run(RJob* job, RTaskScheduler* sched) {
    pthread_mutex_lock(&job->lock);
    if (job->task_count == 0) {
        pthread_mutex_unlock(&job->lock);
        return 0;
    }

    job->status = JOB_RUNNING;
    pthread_mutex_unlock(&job->lock);

    for (int i = 0; i < job->task_count; i++) {
        scheduler_add_task(sched, job->tasks[i]);
    }

    // Return 1: successfully enqueued
    return 1;
}

JobStatus job_status(RJob* job) {
    update_job_status(job);
    pthread_mutex_lock(&job->lock);
    JobStatus s = job->status;
    pthread_mutex_unlock(&job->lock);
    return s;
}
