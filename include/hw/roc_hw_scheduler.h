#pragma once
#include "roc_hw_stats.h"

typedef struct HWTask {
    void (*func)(struct HWTask*); // Task function
    int assigned_core;            // CPU core to pin to
    int is_completed;             // Completion flag
    HANDLE thread_handle;         // Windows thread handle
    int stats_printed;
} HWTask;

typedef struct HWScheduler {
    int num_cores;
    HWTask** tasks;
    int task_count;
    CRITICAL_SECTION lock;        // Windows mutex
} HWScheduler;

// Scheduler API
HWScheduler* hw_create_scheduler(int num_cores);
void hw_scheduler_add_task(HWScheduler* sched, HWTask* task);
void hw_scheduler_start(HWScheduler* sched);
void hw_scheduler_runasync(HWScheduler* sched);
void hw_scheduler_wait(HWScheduler* sched);
void hw_scheduler_destroy(HWScheduler* sched);
