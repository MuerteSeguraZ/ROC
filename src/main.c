#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>                // ✅ for usleep
#include "roc_scheduler.h"
#include "roc_task.h"
#include "roc_bundle.h"
#include "roc_campaign.h"
#include "roc_program.h"
#include "roc_program_queue.h"
#include "roc_thread.h"

int main() {
    printf("=== ROC Multithreaded Program Queue Demo ===\n");

    // --- Scheduler ---
    RTaskScheduler* sched = create_scheduler(4); // 4 resource units
    scheduler_start(sched);                       // start background worker

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

    RProgram* p2 = create_program("Program-Beta", 2);
    program_add_campaign(p2, c2);

    // --- Program Queue ---
    RProgramQueue* pq = create_program_queue("MainQueue");
    program_queue_add(pq, p1);
    program_queue_add(pq, p2);

    // --- Multithreaded Execution ---
    RThread* threads[pq->program_count];
    for (int i = 0; i < pq->program_count; i++) {
        threads[i] = thread_create((void(*)(void*))program_run, pq->programs[i]);
        thread_start(threads[i]);
    }

    // Wait for all threads to finish
    for (int i = 0; i < pq->program_count; i++) {
        thread_join(threads[i]);
        free(threads[i]);
    }

    printf("All programs completed!\n");

    destroy_program_queue(pq);
    scheduler_stop(sched);
    destroy_scheduler(sched);

    return 0;
}
