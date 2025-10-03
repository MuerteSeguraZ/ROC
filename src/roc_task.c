#include "roc_task.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// =====================
// Task operations
// =====================
RTask* create_task(const char* name, int priority) {
    RTask* task = (RTask*)malloc(sizeof(RTask));
    strcpy(task->name, name);
    task->resource_count = 0;
    task->priority = priority;
    task->status = TASK_PENDING;
    pthread_mutex_init(&task->lock, NULL);
    return task;
}

void destroy_task(RTask* task) {
    pthread_mutex_destroy(&task->lock);
    free(task);
}

int add_resource_req(RTask* task, RNode* node, int amount) {
    pthread_mutex_lock(&task->lock);
    if (task->resource_count >= MAX_RESOURCES_PER_TASK) {
        pthread_mutex_unlock(&task->lock);
        return 0;
    }
    task->resources[task->resource_count].node = node;
    task->resources[task->resource_count].amount = amount;
    task->resource_count++;
    pthread_mutex_unlock(&task->lock);
    return 1;
}

int remove_resource_req(RTask* task, int index) {
    pthread_mutex_lock(&task->lock);
    if (index < 0 || index >= task->resource_count) {
        pthread_mutex_unlock(&task->lock);
        return 0;
    }
    for (int i = index; i < task->resource_count - 1; i++)
        task->resources[i] = task->resources[i + 1];
    task->resource_count--;
    pthread_mutex_unlock(&task->lock);
    return 1;
}

// Reserve all resources for a task
int allocate_task(RTask* task) {
    pthread_mutex_lock(&task->lock);
    for (int i = 0; i < task->resource_count; i++) {
        TaskResourceReq* r = &task->resources[i];
        if (!reserve(r->node, r->amount)) {
            // Rollback any previous reservations
            for (int j = 0; j < i; j++)
                release(task->resources[j].node, task->resources[j].amount);
            task->status = TASK_FAILED;
            pthread_mutex_unlock(&task->lock);
            return 0;
        }
    }
    task->status = TASK_RUNNING;
    pthread_mutex_unlock(&task->lock);
    return 1;
}

// Release all resources
void release_task(RTask* task) {
    pthread_mutex_lock(&task->lock);
    for (int i = 0; i < task->resource_count; i++)
        release(task->resources[i].node, task->resources[i].amount);
    task->status = TASK_COMPLETED;
    pthread_mutex_unlock(&task->lock);
}

// =====================
// Internal thread function
// =====================
static void* run_task_thread(void* arg) {
    RTask* task = (RTask*)arg;

    int total_units = 0;
    for (int i = 0; i < task->resource_count; i++)
        total_units += task->resources[i].amount;

    printf("Running task '%s' using %d resource units...\n", task->name, total_units);
    usleep(total_units * 200000); // simulate work

    release_task(task);
    printf("Task '%s' completed.\n", task->name);
    return NULL;
}

// =====================
// Public execution functions
// =====================

// Simulate task execution (blocking)
int run_task(RTask* task) {
    // Reserve resources first
    if (!allocate_task(task)) return 0;

    pthread_t thread;
    if (pthread_create(&thread, NULL, run_task_thread, task) != 0) {
        // Failed to create thread, release resources
        release_task(task);
        return 0;
    }

    pthread_detach(thread); // Let it run independently
    return 1;
}

// Run task asynchronously in a new thread
int run_task_async(RTask* task) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, run_task_thread, task) != 0)
        return 0;
    pthread_detach(thread); // Let it run independently
    return 1;
}

// Check task status
TaskStatus task_status(RTask* task) {
    pthread_mutex_lock(&task->lock);
    TaskStatus s = task->status;
    pthread_mutex_unlock(&task->lock);
    return s;
}
