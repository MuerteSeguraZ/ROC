#include "roc.h"
#include "roc_task.h"
#include "roc_scheduler.h"
#include "roc_bundle.h"
#include "roc_bundle_queue.h"
#include "roc_campaign.h"
#include "roc_campaign_queue.h"
#include "roc_stage.h"
#include "roc_stage_queue.h"
#include "roc_phase.h"
#include "roc_phase_queue.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("=== ROC Full Test Demo ===\n");

    // --- Create nodes ---
    RNode* cpu = create_node("CPU", "CPU", 8);
    RNode* gpu = create_node("GPU", "GPU", 4);
    RNode* mem = create_node("RAM", "Memory", 16);

    // --- Create scheduler ---
    RTaskScheduler* sched = create_scheduler();
    scheduler_start(sched);

    // === 1) Create tasks ===
    RTask* t_cpu1 = create_task("Task-CPU1", 5);
    add_resource_req(t_cpu1, cpu, 2);

    RTask* t_gpu1 = create_task("Task-GPU1", 3);
    add_resource_req(t_gpu1, gpu, 1);

    RTask* t_mem1 = create_task("Task-MEM1", 2);
    add_resource_req(t_mem1, mem, 4);

    // === 2) Create bundles ===
    RBundle* bundle1 = create_bundle("Bundle-Alpha", 10);
    bundle_add_task(bundle1, t_cpu1);

    RBundle* bundle2 = create_bundle("Bundle-Beta", 5);
    bundle_add_task(bundle2, t_gpu1);
    bundle_add_task(bundle2, t_mem1);

    // === 3) Create campaigns ===
    RCampaign* campaign1 = create_campaign("Campaign-Prime", 10);
    campaign_add_bundle(campaign1, bundle1);

    RCampaign* campaign2 = create_campaign("Campaign-Secondary", 5);
    campaign_add_bundle(campaign2, bundle2);

    // === 4) Create campaign queue ===
    RCampaignQueue* cq = create_campaign_queue();
    enqueue_campaign(cq, campaign2); // lower priority first
    enqueue_campaign(cq, campaign1); // higher priority

    // === 5) Create stages ===
    RStage* stage1 = create_stage("Stage-Alpha", 10);
    stage_add_item(stage1, campaign1, STAGE_ITEM_CAMPAIGN);

    RStage* stage2 = create_stage("Stage-Beta", 5);
    stage_add_item(stage2, campaign2, STAGE_ITEM_CAMPAIGN);

    RStageQueue* sq = create_stage_queue("MainStageQueue");
    stage_queue_add(sq, stage2); // stage priorities respected
    stage_queue_add(sq, stage1);

    // === 6) Create phases ===
    RPhase* phase1 = create_phase("Phase-Prime", 10);
    phase_add_stage(phase1, stage1);

    RPhase* phase2 = create_phase("Phase-Secondary", 5);
    phase_add_stage(phase2, stage2);

    RPhaseQueue* pq = create_phase_queue("MainPhaseQueue");
    phase_queue_add(pq, phase2); // lower priority first
    phase_queue_add(pq, phase1);

    // === 7) Run phase queue ===
    printf("\n--- Running Phase Queue ---\n");
    phase_queue_run_priority(pq, sched);

    // --- Cleanup ---
    destroy_task(t_cpu1);
    destroy_task(t_gpu1);
    destroy_task(t_mem1);

    destroy_bundle(bundle1);
    destroy_bundle(bundle2);

    destroy_campaign(campaign1);
    destroy_campaign(campaign2);
    destroy_campaign_queue(cq);

    destroy_stage(stage1);
    destroy_stage(stage2);
    destroy_stage_queue(sq);

    destroy_phase(phase1);
    destroy_phase(phase2);
    destroy_phase_queue(pq);

    scheduler_stop(sched);
    destroy_scheduler(sched);

    destroy_node(cpu);
    destroy_node(gpu);
    destroy_node(mem);

    printf("\n[Main] ROC full test completed.\n");
    return 0;
}
