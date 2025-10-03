#include "roc.h"
#include "roc_task.h"
#include "roc_scheduler.h"
#include "roc_workflow.h"
#include "roc_workflow_queue.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("=== ROC Workflow Queue Demo ===\n");

    // --- Create nodes ---
    RNode* cpu = create_node("CPU", "CPU", 8);
    RNode* gpu = create_node("GPU", "GPU", 4);

    // --- Create scheduler ---
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // --- Create workflow queue ---
    RWorkflowQueue* wf_queue = create_workflow_queue();

    // --- Workflow 1 ---
    RWorkflow* wf1 = create_workflow("Workflow-Alpha", 10);
    RTask* t1 = create_task("Task-1", 5);
    add_resource_req(t1, cpu, 2);
    workflow_add_task(wf1, t1);

    RTask* t2 = create_task("Task-2", 3);
    add_resource_req(t2, gpu, 1);
    workflow_add_task(wf1, t2);

    workflow_queue_add(wf_queue, wf1);

    // --- Workflow 2 ---
    RWorkflow* wf2 = create_workflow("Workflow-Beta", 5);
    RTask* t3 = create_task("Task-3", 4);
    add_resource_req(t3, cpu, 4);
    workflow_add_task(wf2, t3);

    workflow_queue_add(wf_queue, wf2);

    // --- Workflow 3 ---
    RWorkflow* wf3 = create_workflow("Workflow-Gamma", 8);
    RTask* t4 = create_task("Task-4", 2);
    add_resource_req(t4, cpu, 2);
    workflow_add_task(wf3, t4);

    RTask* t5 = create_task("Task-5", 2);
    add_resource_req(t5, gpu, 2);
    workflow_add_task(wf3, t5);

    workflow_queue_add(wf_queue, wf3);

    // --- Run all workflows in queue ---
    workflow_queue_run(wf_queue, sched);

    // --- Wait until all workflows complete ---
    while (!workflow_queue_all_completed(wf_queue)) {
        usleep(50000);
    }

    printf("[Main] All workflows in queue completed\n");

    // --- Cleanup ---
    destroy_task(t1);
    destroy_task(t2);
    destroy_task(t3);
    destroy_task(t4);
    destroy_task(t5);

    destroy_workflow(wf1);
    destroy_workflow(wf2);
    destroy_workflow(wf3);

    destroy_workflow_queue(wf_queue);
    scheduler_stop(sched);
    destroy_scheduler(sched);

    destroy_node(cpu);
    destroy_node(gpu);

    return 0;
}
