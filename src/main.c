#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include "hw/roc_hw_scheduler.h"

// Example task function
void test_task(HWTask* task) {
    printf(" -> Running task on logical core %d (priority=%d)\n",
           task->assigned_core, task->priority);
    // Simulate work
    for (volatile long i = 0; i < 50000000; i++);
}

int main() {
    printf("Starting scheduler priority test...\n");

    // Create scheduler (detect logical cores automatically)
    HWScheduler* sched = hw_create_scheduler(0);

    printf("System reports %d logical cores.\n", sched->num_logical_cores);

    // Add tasks with different priorities
    int priorities[] = {0, 0, 5, 10, -5, -10};
    for (int i = 0; i < 6; i++) {
        HWTask* task = calloc(1, sizeof(HWTask));
        task->func = test_task;
        task->assigned_core = -1;
        task->priority = priorities[i];
        hw_scheduler_add_task(sched, task);
    }

    printf("Starting tasks...\n");
    hw_scheduler_start(sched);

    hw_scheduler_wait(sched);
    hw_scheduler_destroy(sched);

    printf("All tasks finished.\n");
    return 0;
}
