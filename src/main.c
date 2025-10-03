#include "roc.h"
#include "roc_task.h"
#include "roc_scheduler.h"
#include "roc_job.h"
#include "roc_workflow.h"
#include "roc_stage.h"
#include "roc_stage_queue.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    RNode* cpu = create_node("CPU", "CPU", 8);
    RNode* gpu = create_node("GPU", "GPU", 4);

    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // --- Stage 1 ---
    RStage* stage1 = create_stage("Stage-Alpha", 10);
    RJob* job1 = create_job("Job-1", 5);
    RTask* jt1 = create_task("Job-Task1", 2);
    add_resource_req(jt1, cpu, 2);
    job_add_task(job1, jt1);
    stage_add_item(stage1, job1, STAGE_ITEM_JOB);

    RWorkflow* wf1 = create_workflow("Workflow-Beta", 5);
    RTask* wt1 = create_task("Workflow-Task1", 1);
    add_resource_req(wt1, gpu, 1);
    workflow_add_task(wf1, wt1);
    stage_add_item(stage1, wf1, STAGE_ITEM_WORKFLOW);

    // --- Stage 2 ---
    RStage* stage2 = create_stage("Stage-Beta", 5);
    RJob* job2 = create_job("Job-2", 3);
    RTask* jt2 = create_task("Job-Task2", 2);
    add_resource_req(jt2, cpu, 2);
    job_add_task(job2, jt2);
    stage_add_item(stage2, job2, STAGE_ITEM_JOB);

    // --- Create Stage Queue ---
    RStageQueue* queue = create_stage_queue("MainQueue");
    stage_queue_add(queue, stage1);
    stage_queue_add(queue, stage2);

    printf("=== Running stages in queue ===\n");
    stage_queue_run_priority(queue, sched);

    printf("[Main] All stages in queue completed\n");

    // Cleanup
    destroy_task(jt1);
    destroy_task(jt2);
    destroy_task(wt1);
    destroy_job(job1);
    destroy_job(job2);
    destroy_workflow(wf1);
    destroy_stage(stage1);
    destroy_stage(stage2);
    destroy_stage_queue(queue);
    scheduler_stop(sched);
    destroy_scheduler(sched);
    destroy_node(cpu);
    destroy_node(gpu);

    return 0;
}
