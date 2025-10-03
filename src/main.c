#include "roc.h"
#include "roc_task.h"
#include "roc_scheduler.h"
#include "roc_pipe.h"
#include "roc_pipe_queue.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    // --- Nodes ---
    RNode* cpu = create_node("CPU", "CPU", 8);
    RNode* gpu = create_node("GPU", "GPU", 4);

    // --- Scheduler ---
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // -----------------------------
    // Individual Pipes
    // -----------------------------
    printf("=== Running individual pipes ===\n");
    RPipe* pipe1 = create_pipe("Pipe-Alpha");
    RTask* t1 = create_task("Task-1", 2);
    add_resource_req(t1, cpu, 2);
    pipe_add_task(pipe1, t1);

    RTask* t2 = create_task("Task-2", 1);
    add_resource_req(t2, gpu, 1);
    pipe_add_task(pipe1, t2);

    pipe_run(pipe1, sched);

    // Wait for completion
    while (pipe_status(pipe1) != PIPE_COMPLETED) {
        usleep(50000);
    }
    printf("[Main] Individual pipe 'Pipe-Alpha' completed\n");

    // -----------------------------
    // Pipe Queue
    // -----------------------------
    printf("=== Running pipes in a queue ===\n");
    RPipe* pipe2 = create_pipe("Pipe-Beta");
    RTask* t3 = create_task("Task-3", 2);
    add_resource_req(t3, cpu, 4);
    pipe_add_task(pipe2, t3);

    RPipe* pipe3 = create_pipe("Pipe-Gamma");
    RTask* t4 = create_task("Task-4", 2);
    add_resource_req(t4, gpu, 2);
    pipe_add_task(pipe3, t4);

    RPipeQueue* pq = create_pipe_queue("MainQueue");
    pipe_queue_add(pq, pipe2);
    pipe_queue_add(pq, pipe3);

    pipe_queue_run(pq, sched);

    // Wait for queue completion
    while (pipe_queue_status(pq) != PIPEQUEUE_COMPLETED) {
        usleep(50000);
    }

    printf("[Main] Pipe queue 'MainQueue' completed\n");

    // --- Cleanup ---
    destroy_task(t1);
    destroy_task(t2);
    destroy_task(t3);
    destroy_task(t4);

    destroy_pipe(pipe1);
    destroy_pipe(pipe2);
    destroy_pipe(pipe3);
    destroy_pipe_queue(pq);

    scheduler_stop(sched);
    destroy_scheduler(sched);

    destroy_node(cpu);
    destroy_node(gpu);

    return 0;
}
