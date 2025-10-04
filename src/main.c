#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "hw/roc_hw_scheduler.h"
#include "hw/hw_utils.h"

// Example task function
void test_task(HWTask* task) {
    printf(" -> Running task on logical core %d (priority=%d, quantum=%d ms)\n",
           task->assigned_core, task->priority, task->quantum_ms);

    // Simulate some work
    for (volatile long i = 0; i < 100000000; i++);

    // Optionally yield to allow other threads to run
    hw_yield_task(task);
}

int main() {
    printf("Starting scheduler time-slice test...\n");

    // Create scheduler (detect logical cores automatically)
    HWScheduler* sched = hw_create_scheduler(0);

    printf("System reports %d logical cores.\n", sched->num_logical_cores);

    // Add tasks with different priorities and quantum times
    int priorities[] = {10, 5, 0, 0, -5, -10};
    int quantums[]   = {100, 150, 200, 50, 120, 80}; // in milliseconds

    for (int i = 0; i < 6; i++) {
        HWTask* task = calloc(1, sizeof(HWTask));
        task->func = test_task;
        task->assigned_core = -1;
        task->priority = priorities[i];
        task->quantum_ms = quantums[i]; // set per-task quantum
        hw_scheduler_add_task(sched, task);
    }

    printf("Starting tasks...\n");
    hw_scheduler_start(sched);

    hw_scheduler_wait(sched);
    hw_scheduler_destroy(sched);

    printf("All tasks finished.\n");
    return 0;
}
