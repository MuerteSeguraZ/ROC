#include "roc_pipe_queue.h"
#include "roc_pipe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// =====================
// Pipe Queue Operations
// =====================
RPipeQueue* create_pipe_queue(const char* name) {
    RPipeQueue* queue = (RPipeQueue*)malloc(sizeof(RPipeQueue));
    strcpy(queue->name, name);
    queue->pipe_count = 0;
    queue->status = PIPEQUEUE_PENDING;
    pthread_mutex_init(&queue->lock, NULL);
    return queue;
}

void destroy_pipe_queue(RPipeQueue* queue) {
    pthread_mutex_destroy(&queue->lock);
    free(queue);
}

int pipe_queue_add(RPipeQueue* queue, RPipe* pipe) {
    pthread_mutex_lock(&queue->lock);
    if (queue->pipe_count >= MAX_PIPES_PER_QUEUE) {
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }
    queue->pipes[queue->pipe_count++] = pipe;
    pthread_mutex_unlock(&queue->lock);
    return 1;
}

static int compare_pipe_priority(const void* a, const void* b) {
    RPipe* p1 = *(RPipe**)a;
    RPipe* p2 = *(RPipe**)b;
    return p2->priority - p1->priority; // descending
}

void run_pipe_queue_priority(RPipeQueue* queue, RTaskScheduler* sched) {
    pthread_mutex_lock(&queue->lock);
    qsort(queue->pipes, queue->pipe_count, sizeof(RPipe*), compare_pipe_priority);
    pthread_mutex_unlock(&queue->lock);

    for (int i = 0; i < queue->pipe_count; i++) {
        pipe_run(queue->pipes[i], sched);
        while (pipe_status(queue->pipes[i]) != PIPE_COMPLETED) {
            usleep(50000);
        }
        printf("[Queue] Pipe '%s' completed\n", queue->pipes[i]->name);
    }
}

// =====================
// Internal thread for running the queue sequentially
// =====================
static void* run_pipe_queue_thread(void* arg) {
    RPipeQueue* queue = (RPipeQueue*)arg;

    pthread_mutex_lock(&queue->lock);
    queue->status = PIPEQUEUE_RUNNING;
    pthread_mutex_unlock(&queue->lock);

    for (int i = 0; i < queue->pipe_count; i++) {
        pipe_run(queue->pipes[i], NULL); // NULL scheduler will just enqueue tasks in the default way

        // Wait for pipe to complete
        while (pipe_status(queue->pipes[i]) != PIPE_COMPLETED) {
            usleep(50000);
        }
    }

    pthread_mutex_lock(&queue->lock);
    queue->status = PIPEQUEUE_COMPLETED;
    pthread_mutex_unlock(&queue->lock);

    printf("[Main] All pipes in queue '%s' completed\n", queue->name);
    return NULL;
}

// =====================
// Public execution functions
// =====================
int pipe_queue_run(RPipeQueue* queue, RTaskScheduler* sched) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, run_pipe_queue_thread, queue) != 0) {
        return 0;
    }
    pthread_detach(thread);
    return 1;
}

PipeQueueStatus pipe_queue_status(RPipeQueue* queue) {
    pthread_mutex_lock(&queue->lock);
    PipeQueueStatus s = queue->status;
    pthread_mutex_unlock(&queue->lock);
    return s;
}
