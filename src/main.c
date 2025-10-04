#include "hw/roc_hw_scheduler.h"
#include <windows.h>
#include <stdio.h>

// Dummy task function
void my_task(HWTask* task) {
    printf(" -> Running task on logical core %d\n", task->assigned_core);
    // Simulate some CPU + memory load
    volatile unsigned long long sum = 0;
    for (unsigned long long i = 0; i < 50000000ULL; i++) {
        sum += i;
    }
    Sleep(100); // add some wait so we see context
    (void)sum;  // prevent optimization
}

int main(void) {
    SYSTEM_INFO si;
    GetSystemInfo(&si);

    printf("System reports %u logical processors.\n", si.dwNumberOfProcessors);

    // Create scheduler
    HWScheduler* sched = hw_create_scheduler(si.dwNumberOfProcessors);

    // Add N tasks (e.g., 2x number of logical processors)
    int num_tasks = si.dwNumberOfProcessors * 2;
    for (int i = 0; i < num_tasks; i++) {
        HWTask* task = (HWTask*)calloc(1, sizeof(HWTask));
        task->func = my_task;
        task->assigned_core = -1;
        task->stats_printed = 0;
        task->is_completed = 0;
        hw_scheduler_add_task(sched, task);
    }

    printf("Starting %d tasks...\n", num_tasks);
    hw_scheduler_start(sched);

    // Wait for all tasks
    hw_scheduler_wait(sched);

    // Clean up
    hw_scheduler_destroy(sched);

    printf("All tasks finished.\n");
    return 0;
}
