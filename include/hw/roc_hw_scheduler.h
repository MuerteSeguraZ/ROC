#pragma once
#include "roc_hw_stats.h"
#include <windows.h>

// Forward declare HWScheduler so HWTask can reference it
typedef struct HWScheduler HWScheduler;

typedef struct HWTask {
    void (*func)(struct HWTask* task);
    int assigned_core;           // initialize to -1
    int target_core;
    int stats_printed;           // initialize to 0
    int is_completed;            // initialize to 0
    int priority;                // NEW: higher = more important
    HANDLE thread_handle;
    HWScheduler* scheduler;      // link back to scheduler
    uint32_t quantum_ms;
    uint64_t last_start_time;
    int yield_requested;  
} HWTask;

struct HWScheduler {
    int num_logical_cores;
    int num_physical_cores;
    int* logical_to_physical;
    HWTask** tasks;
    int task_count;
    int* tasks_per_core;
    int* completed_tasks_per_core;
    CRITICAL_SECTION lock;
};

// Scheduler API
HWScheduler* hw_create_scheduler(int num_cores);
void hw_scheduler_add_task(HWScheduler* sched, HWTask* task);
void hw_scheduler_start(HWScheduler* sched);
void hw_scheduler_runasync(HWScheduler* sched);
void hw_scheduler_wait(HWScheduler* sched);
void hw_scheduler_destroy(HWScheduler* sched);
