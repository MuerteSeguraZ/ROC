#ifndef ROC_PHASE_QUEUE_H
#define ROC_PHASE_QUEUE_H

#include "roc_phase.h"
#include "roc_scheduler.h"

#define MAX_PHASES_PER_QUEUE 16

typedef struct {
    char name[50];
    RPhase* phases[MAX_PHASES_PER_QUEUE];
    int phase_count;
} RPhaseQueue;

// PhaseQueue operations
RPhaseQueue* create_phase_queue(const char* name);
void destroy_phase_queue(RPhaseQueue* queue);

int phase_queue_add(RPhaseQueue* queue, RPhase* phase);
int phase_queue_run(RPhaseQueue* queue, RTaskScheduler* sched); // runs all phases in order

int phase_queue_run_priority(RPhaseQueue* queue, RTaskScheduler* sched); // run by descending priority

#endif
