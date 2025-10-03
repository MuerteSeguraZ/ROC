#include "include/roc.h"
#include "include/roc_task.h"
#include "include/roc_scheduler.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("=== ROC Task Scheduler Demo ===\n");

    // --- Create nodes properly ---
    RNode* cpu1 = create_node("CPU1", "CPU", 8);
    RNode* cpu2 = create_node("CPU2", "CPU", 4);
    RNode* gpu1 = create_node("GPU1", "GPU", 2);

    printf("Nodes in network (3):\n");
    printf(" - %s (CPU, %d/%d)\n", cpu1->name, cpu1->available, cpu1->capacity);
    printf(" - %s (CPU, %d/%d)\n", cpu2->name, cpu2->available, cpu2->capacity);
    printf(" - %s (GPU, %d/%d)\n", gpu1->name, gpu1->available, gpu1->capacity);

    // --- Create scheduler ---
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // --- Create tasks ---
    RTask* t1 = create_task("Task-Heavy", 10);
    add_resource_req(t1, cpu1, 6);

    RTask* t2 = create_task("Task-Medium", 5);
    add_resource_req(t2, cpu2, 3);
    add_resource_req(t2, gpu1, 1);

    RTask* t3 = create_task("Task-Light", 1);
    add_resource_req(t3, cpu1, 2);

    // --- Add tasks to scheduler ---
    scheduler_add_task(sched, t1);
    scheduler_add_task(sched, t2);
    scheduler_add_task(sched, t3);

    printf("[Main] Tasks added to scheduler.\n");

    // Wait for tasks to complete
    int done = 0;
    while (!done) {
        done = 1;
        if (task_status(t1) != TASK_COMPLETED) done = 0;
        if (task_status(t2) != TASK_COMPLETED) done = 0;
        if (task_status(t3) != TASK_COMPLETED) done = 0;
        usleep(50000);
    }

    // Cleanup
    destroy_task(t1);
    destroy_task(t2);
    destroy_task(t3);
    scheduler_stop(sched);
    destroy_scheduler(sched);

    // Destroy nodes
    destroy_node(cpu1);
    destroy_node(cpu2);
    destroy_node(gpu1);

    printf("[Main] All tasks completed.\n");
    return 0;
}
