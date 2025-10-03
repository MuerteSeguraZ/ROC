#include "roc.h"
#include "roc_task.h"
#include "roc_scheduler.h"
#include "roc_job.h"
#include "roc_job_queue.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("=== ROC Job Queue Demo ===\n");

    // Create nodes
    RNode* cpu1 = create_node("CPU1", "CPU", 8);
    RNode* gpu1 = create_node("GPU1", "GPU", 4);

    // Scheduler
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // Job queue
    RJobQueue* jq = create_job_queue();

    // --- Job 1 ---
    RJob* job1 = create_job("Job-Alpha", 5);
    RTask* t1a = create_task("Task-A1", 10);
    add_resource_req(t1a, cpu1, 4);
    job_add_task(job1, t1a);

    RTask* t1b = create_task("Task-A2", 8);
    add_resource_req(t1b, gpu1, 2);
    job_add_task(job1, t1b);

    job_queue_add(jq, job1);

    // --- Job 2 (higher priority) ---
    RJob* job2 = create_job("Job-Beta", 10);
    RTask* t2a = create_task("Task-B1", 6);
    add_resource_req(t2a, cpu1, 2);
    job_add_task(job2, t2a);

    job_queue_add(jq, job2);

    // Process job queue
    RJob* job;
    while ((job = job_queue_next(jq)) != NULL) {
        printf("[Main] Starting job '%s'\n", job->name);
        job_run(job, sched);

        // Wait for completion
        while (job_status(job) != JOB_COMPLETED) {
            usleep(50000);
        }
        printf("[Main] Job '%s' completed.\n", job->name);
        destroy_job(job);
    }

    // Cleanup
    scheduler_stop(sched);
    destroy_scheduler(sched);
    destroy_job_queue(jq);
    destroy_node(cpu1);
    destroy_node(gpu1);

    printf("[Main] All jobs processed.\n");
    return 0;
}
