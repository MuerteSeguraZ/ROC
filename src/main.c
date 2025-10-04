#include <stdio.h>
#include <windows.h>
#include "hw/roc_hw_scheduler.h" 

#define NUM_CORES 4
#define NUM_TASKS 12

// Simple task: just sleep a bit and print stats
void sample_task(HWTask* task) {
    Sleep(100 + (rand() % 100)); // simulate work
}

int main() {
    printf("=== ROC HW Layer Static Affinity Test ===\n");

    HWScheduler* sched = hw_create_scheduler(NUM_CORES);

    HWTask* tasks[NUM_TASKS];

    // Create tasks with no assigned core (-1) to test load balancing
    for (int i = 0; i < NUM_TASKS; i++) {
        tasks[i] = malloc(sizeof(HWTask));
        tasks[i]->assigned_core = -1; // let scheduler pick least-loaded core
        tasks[i]->stats_printed = 0;
        tasks[i]->is_completed = 0;
        tasks[i]->func = sample_task;
        hw_scheduler_add_task(sched, tasks[i]);
    }

    // Start all tasks
    hw_scheduler_start(sched);   

    // Wait for all tasks to finish
    for (int i = 0; i < NUM_TASKS; i++) {
        WaitForSingleObject(tasks[i]->thread_handle, INFINITE);
    }

    printf("\nTask assignment per core:\n");
    for (int i = 0; i < NUM_CORES; i++) {
        printf("Core %d handled approx %d tasks\n", i, sched->completed_tasks_per_core[i]);
    }

    // Cleanup
    for (int i = 0; i < NUM_TASKS; i++) free(tasks[i]);
    free(sched->tasks_per_core);
    free(sched->tasks);
    hw_scheduler_destroy(sched);

    printf("\n=== Test Completed ===\n");
    return 0;
}