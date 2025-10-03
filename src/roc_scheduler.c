#include "roc_scheduler.h"
#include "roc_task.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// =====================
// Internal helper: find the highest priority pending task
// =====================
static int find_highest_priority(RTaskScheduler* sched) {
    int idx = -1;
    int max_priority = -1;
    for (int i = 0; i < sched->count; i++) {
        if (sched->queue[i]->status == TASK_PENDING && sched->queue[i]->priority > max_priority) {
            max_priority = sched->queue[i]->priority;
            idx = i;
        }
    }
    return idx;
}

// =====================
// Scheduler thread
// =====================
static void* scheduler_thread(void* arg) {
    RTaskScheduler* sched = (RTaskScheduler*)arg;

    while (sched->running) {
        pthread_mutex_lock(&sched->lock);
        while (sched->count == 0 && sched->running)
            pthread_cond_wait(&sched->cond, &sched->lock);

        int idx = find_highest_priority(sched);
        if (idx != -1) {
            RTask* task = sched->queue[idx];

            // Remove task from queue
            for (int i = idx; i < sched->count - 1; i++)
                sched->queue[i] = sched->queue[i + 1];
            sched->count--;

            pthread_mutex_unlock(&sched->lock);

            // Run task in its own thread (allocates resources internally)
            if (!run_task(task)) {
                printf("[Scheduler] Failed to run task '%s'.\n", task->name);
            }

        } else {
            pthread_mutex_unlock(&sched->lock);
            usleep(10000); // 10ms idle
        }
    }
    return NULL;
}

// =====================
// Scheduler API
// =====================
RTaskScheduler* create_scheduler() {
    RTaskScheduler* sched = (RTaskScheduler*)malloc(sizeof(RTaskScheduler));
    sched->count = 0;
    sched->running = 0;
    pthread_mutex_init(&sched->lock, NULL);
    pthread_cond_init(&sched->cond, NULL);
    return sched;
}

void destroy_scheduler(RTaskScheduler* sched) {
    scheduler_stop(sched);
    pthread_mutex_destroy(&sched->lock);
    pthread_cond_destroy(&sched->cond);
    free(sched);
}

int scheduler_add_task(RTaskScheduler* sched, RTask* task) {
    pthread_mutex_lock(&sched->lock);
    if (sched->count >= MAX_TASK_QUEUE) {
        pthread_mutex_unlock(&sched->lock);
        return 0;
    }
    sched->queue[sched->count++] = task;
    pthread_cond_signal(&sched->cond);
    pthread_mutex_unlock(&sched->lock);
    return 1;
}

void scheduler_start(RTaskScheduler* sched) {
    pthread_mutex_lock(&sched->lock);
    if (!sched->running) {
        sched->running = 1;
        pthread_create(&sched->thread, NULL, scheduler_thread, sched);
    }
    pthread_mutex_unlock(&sched->lock);
}

void scheduler_stop(RTaskScheduler* sched) {
    pthread_mutex_lock(&sched->lock);
    if (sched->running) {
        sched->running = 0;
        pthread_cond_signal(&sched->cond);
        pthread_mutex_unlock(&sched->lock);
        pthread_join(sched->thread, NULL);
    } else {
        pthread_mutex_unlock(&sched->lock);
    }
}
