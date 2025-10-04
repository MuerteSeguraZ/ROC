#include "hw/roc_hw_scheduler.h"
#include <windows.h>
#include <stdio.h>

void sample_task(HWTask* task) {
    printf("Task starting on core %d\n", task->assigned_core);
    
    volatile long sum = 0;
    for (long i = 0; i < 100000000; i++) {
        sum += i;
    }

    printf("Task finished on core %d\n", task->assigned_core);
}

int main() {
    HWScheduler* sched = hw_create_scheduler(4);

    // Create 4 tasks, one per core
    for (int i = 0; i < 4; i++) {
        HWTask* t = malloc(sizeof(HWTask));
        t->func = sample_task;
        t->assigned_core = i;
        t->is_completed = 0;
        t->thread_handle = NULL;
        hw_scheduler_add_task(sched, t);
    }

    // Run tasks asynchronously
    hw_scheduler_runasync(sched);

    // Poll for task completion and print stats
    int done = 0;
    while (!done) {
        done = 1;
        for (int i = 0; i < sched->task_count; i++) {
            if (!sched->tasks[i]->is_completed) {
                done = 0;
            } else {
                hw_task_stats_t stats = hw_get_task_stats(sched->tasks[i]->thread_handle);
                printf(
                    "Task on core %d: CPU time = %llu ms, Memory = %llu bytes\n",
                    sched->tasks[i]->assigned_core, stats.cpu_time_ms, stats.memory_usage_bytes
                );
            }
        }
        Sleep(100); // Poll interval
    }

    // Clean up
    hw_scheduler_wait(sched);
    hw_scheduler_destroy(sched);
    return 0;
}
