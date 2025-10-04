#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "hw/roc_hw_scheduler.h"
#include "hw/hw_utils.h"

// Example task function
void test_task(HWTask* task) {
    printf(" -> Running task on logical core %d (priority=%d, quantum=%llu ms)\n",
           task->assigned_core, task->priority, task->quantum_ms);

    for (volatile long i = 0; i < 100000000; i++);

    hw_yield_task(task);

    if (task->priority >= 10 && task->assigned_core != 0) {
        task->target_core = 0; 
    }

    // mark task as done after one iteration
    task->is_completed = 1;
}


int main() {
    printf("Starting scheduler time-slice & migration test...\n");

    // Create scheduler (detect logical cores automatically)
    HWScheduler* sched = hw_create_scheduler(0);
    printf("System reports %d logical cores.\n", sched->num_logical_cores);

    // Add tasks with different priorities and quantum times
    int priorities[] = {10, 5, 0, 0, -5, -10};
    int quantums[]   = {100, 150, 200, 50, 120, 80}; // in milliseconds
    HWTask* task_list[6]; // local array to keep track of tasks

    for (int i = 0; i < 6; i++) {
        HWTask* task = calloc(1, sizeof(HWTask));
        task->func = test_task;
        task->assigned_core = -1;
        task->priority = priorities[i];
        task->quantum_ms = quantums[i];
        task->target_core = -1; // no migration initially
        hw_scheduler_add_task(sched, task);
        task_list[i] = task;    // store reference
    }

    printf("Starting tasks...\n");
    hw_scheduler_start(sched);

    // Example: dynamic migration after some time
    Sleep(200); // let some tasks run
    printf(" -> Triggering migration: moving lowest-priority task to core 1\n");
    for (int i = 0; i < 6; i++) {
        if (task_list[i]->priority <= -5) {
            task_list[i]->target_core = 1; // migrate to logical core 1
        }
    }

    hw_scheduler_wait(sched);
    hw_scheduler_destroy(sched);

    printf("All tasks finished.\n");
    return 0;
}
