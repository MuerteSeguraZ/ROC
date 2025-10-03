#include "roc.h"
#include "roc_task.h"
#include "roc_scheduler.h"
#include "roc_stage.h"
#include "roc_stage_queue.h"
#include "roc_phase.h"
#include "roc_phase_queue.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    // --- Create nodes ---
    RNode* cpu = create_node("CPU", "CPU", 8);
    RNode* gpu = create_node("GPU", "GPU", 4);

    // --- Scheduler ---
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // --- Phase 1 ---
    RPhase* phase1 = create_phase("Phase-Prime", 10);
    RTask* t1 = create_task("Job-Task1", 5);
    add_resource_req(t1, cpu, 2);
    phase_add_task(phase1, t1);

    // --- Phase 2 ---
    RPhase* phase2 = create_phase("Phase-Secondary", 5);
    RTask* t2 = create_task("Workflow-Task1", 3);
    add_resource_req(t2, gpu, 1);
    phase_add_task(phase2, t2);

    // --- Phase Queue ---
    RPhaseQueue* queue = create_phase_queue("MainPhaseQueue");
    phase_queue_add(queue, phase1);
    phase_queue_add(queue, phase2);

    // --- Run queue by priority ---
    phase_queue_run_priority(queue, sched);

    // --- Cleanup ---
    destroy_task(t1);
    destroy_task(t2);
    destroy_phase_queue(queue);
    scheduler_stop(sched);
    destroy_scheduler(sched);
    destroy_node(cpu);
    destroy_node(gpu);

    return 0;
}
