#pragma once
#include "roc_hw_stats.h"
#include <windows.h>

// Forward declare HWScheduler so HWTask can reference it
typedef struct HWScheduler HWScheduler;

typedef struct HWTask {
    void (*func)(struct HWTask* task);
    int assigned_core;           // initialize to -1
    int stats_printed;           // initialize to 0
    int is_completed;            // initialize to 0
    HANDLE thread_handle;
    HWScheduler* scheduler;      // link back to scheduler
} HWTask;

struct HWScheduler {
    int num_cores;
    HWTask** tasks;
    int task_count;
    int* tasks_per_core;             // currently running tasks
    int* completed_tasks_per_core;   // total tasks finished per core
    CRITICAL_SECTION lock;
};

// Scheduler API
HWScheduler* hw_create_scheduler(int num_cores);
void hw_scheduler_add_task(HWScheduler* sched, HWTask* task);
void hw_scheduler_start(HWScheduler* sched);
void hw_scheduler_runasync(HWScheduler* sched);
void hw_scheduler_wait(HWScheduler* sched);
void hw_scheduler_destroy(HWScheduler* sched);
