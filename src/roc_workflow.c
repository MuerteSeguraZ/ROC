#include <windows.h>
#include "roc_workflow.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

RWorkflow* create_workflow(const char* name, int priority) {
    RWorkflow* wf = (RWorkflow*)malloc(sizeof(RWorkflow));
    strcpy(wf->name, name);
    wf->task_count = 0;
    wf->priority = priority;
    wf->status = WORKFLOW_PENDING;
    pthread_mutex_init(&wf->lock, NULL);
    return wf;
}

void destroy_workflow(RWorkflow* wf) {
    pthread_mutex_destroy(&wf->lock);
    free(wf);
}

int workflow_add_task(RWorkflow* wf, RTask* task) {
    pthread_mutex_lock(&wf->lock);
    if (wf->task_count >= MAX_TASKS_PER_WORKFLOW) {
        pthread_mutex_unlock(&wf->lock);
        return 0;
    }
    wf->tasks[wf->task_count++] = task;
    pthread_mutex_unlock(&wf->lock);
    return 1;
}

// Internal thread function for workflow
static void* run_workflow_thread(void* arg) {
    RWorkflow* wf = (RWorkflow*)arg;
    pthread_mutex_lock(&wf->lock);
    wf->status = WORKFLOW_RUNNING;
    pthread_mutex_unlock(&wf->lock);

    printf("[Workflow] Starting workflow '%s'\n", wf->name);

    pthread_t threads[MAX_TASKS_PER_WORKFLOW];

    // Launch all tasks asynchronously
    for (int i = 0; i < wf->task_count; i++) {
        if (!run_task_async(wf->tasks[i])) {
            pthread_mutex_lock(&wf->lock);
            wf->status = WORKFLOW_FAILED;
            pthread_mutex_unlock(&wf->lock);
            return NULL;
        }
    }

    // Wait for all tasks to complete
    int done = 0;
    while (!done) {
        done = 1;
        for (int i = 0; i < wf->task_count; i++) {
            if (task_status(wf->tasks[i]) != TASK_COMPLETED) {
                done = 0;
                break;
            }
        }
        #ifdef _WIN32
          Sleep(50);  // milliseconds
        #else
          usleep(50000); // microseconds
        #endif
    }

    pthread_mutex_lock(&wf->lock);
    wf->status = WORKFLOW_COMPLETED;
    pthread_mutex_unlock(&wf->lock);

    printf("[Workflow] Workflow '%s' completed\n", wf->name);
    return NULL;
}

// Run workflow asynchronously
int workflow_run(RWorkflow* wf, RTaskScheduler* sched) {
    pthread_t thread;
    if (pthread_create(&thread, NULL, run_workflow_thread, wf) != 0)
        return 0;
    pthread_detach(thread);
    return 1;
}

WorkflowStatus workflow_status(RWorkflow* wf) {
    pthread_mutex_lock(&wf->lock);
    WorkflowStatus s = wf->status;
    pthread_mutex_unlock(&wf->lock);
    return s;
}
