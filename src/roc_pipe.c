#include "roc_pipe.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Create a new pipe
RPipe* create_pipe(const char* name, int priority) {
    RPipe* pipe = (RPipe*)malloc(sizeof(RPipe));
    strcpy(pipe->name, name);
    pipe->task_count = 0;
    pipe->priority = priority;
    pipe->status = PIPE_PENDING;
    pthread_mutex_init(&pipe->lock, NULL);
    return pipe;
}

// Destroy a pipe
void destroy_pipe(RPipe* pipe) {
    pthread_mutex_destroy(&pipe->lock);
    free(pipe);
}

// Add a task to the pipe
int pipe_add_task(RPipe* pipe, RTask* task) {
    pthread_mutex_lock(&pipe->lock);
    if (pipe->task_count >= MAX_TASKS_PER_PIPE) {
        pthread_mutex_unlock(&pipe->lock);
        return 0;
    }
    pipe->tasks[pipe->task_count++] = task;
    pthread_mutex_unlock(&pipe->lock);
    return 1;
}

// Internal thread function to run tasks sequentially
static void* run_pipe_thread(void* arg) {
    RPipe* pipe = (RPipe*)arg;

    pthread_mutex_lock(&pipe->lock);
    pipe->status = PIPE_RUNNING;
    pthread_mutex_unlock(&pipe->lock);

    for (int i = 0; i < pipe->task_count; i++) {
        if (!run_task(pipe->tasks[i])) {
            pthread_mutex_lock(&pipe->lock);
            pipe->status = PIPE_FAILED;
            pthread_mutex_unlock(&pipe->lock);
            return NULL;
        }

        // Wait for the task to complete before continuing
        while (task_status(pipe->tasks[i]) != TASK_COMPLETED) {
            usleep(50000);
        }
    }

    pthread_mutex_lock(&pipe->lock);
    pipe->status = PIPE_COMPLETED;
    pthread_mutex_unlock(&pipe->lock);

    printf("[Pipe] Pipe '%s' completed\n", pipe->name);
    return NULL;
}

// Run the pipe asynchronously
int pipe_run(RPipe* pipe, RTaskScheduler* sched) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, run_pipe_thread, pipe) != 0)
        return 0;

    pthread_detach(thread);
    return 1;
}

// Check pipe status
PipeStatus pipe_status(RPipe* pipe) {
    pthread_mutex_lock(&pipe->lock);
    PipeStatus s = pipe->status;
    pthread_mutex_unlock(&pipe->lock);
    return s;
}
