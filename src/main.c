#include <stdio.h>
#include <unistd.h>              // ✅ for usleep
#include "roc_scheduler.h"
#include "roc_task.h"
#include "roc_bundle.h"
#include "roc_campaign.h"
#include "roc_program.h"
#include "roc_program_queue.h"

int main() {
    printf("=== ROC Program Queue Demo ===\n");

    // Create scheduler
    RTaskScheduler* sched = create_scheduler(4); // 4 resource units
    scheduler_start(sched);                       // ✅ Start background worker

    // --- Tasks ---
    RTask* t1 = create_task("Task-CPU1", 2);
    RTask* t2 = create_task("Task-CPU2", 1);
    RTask* t3 = create_task("Task-GPU1", 1);

    // --- Bundles ---
    RBundle* b1 = create_bundle("Bundle-Alpha", 1);
    bundle_add_task(b1, t1);

    RBundle* b2 = create_bundle("Bundle-Beta", 2);
    bundle_add_task(b2, t2);

    RBundle* b3 = create_bundle("Bundle-Gamma", 1);
    bundle_add_task(b3, t3);

    // --- Campaigns ---
    RCampaign* c1 = create_campaign("Campaign-A", 1);
    campaign_add_bundle(c1, b1);
    campaign_add_bundle(c1, b2);

    RCampaign* c2 = create_campaign("Campaign-B", 2);
    campaign_add_bundle(c2, b3);

    // --- Programs ---
    RProgram* p1 = create_program("Program-Alpha", 1);
    program_add_campaign(p1, c1);

    RProgram* p2 = create_program("Program-Beta", 2); // higher priority
    program_add_campaign(p2, c2);

    // --- Program Queue ---
    RProgramQueue* pq = create_program_queue("MainProgramQueue");
    program_queue_add(pq, p1);
    program_queue_add(pq, p2);

    // Run queue sequentially
    program_queue_run(pq, sched);

    // Run queue by priority
    printf("\n--- Running with priority order ---\n");
    program_queue_run_priority(pq, sched);

    // Wait for all programs in queue to complete
    int done = 0;
    while (!done) {
        done = 1;
        for (int i = 0; i < pq->program_count; i++) {
            if (program_status(pq->programs[i]) != PROGRAM_COMPLETED) {
                done = 0;
                break;
            }
        }
        usleep(50000); // let scheduler catch up
    }

    destroy_program_queue(pq);
    scheduler_stop(sched);
    destroy_scheduler(sched);

    return 0;
}
