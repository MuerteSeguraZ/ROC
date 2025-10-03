#ifndef ROC_PHASE_H
#define ROC_PHASE_H

#include "roc_stage.h"
#include "roc_scheduler.h"
#include <pthread.h>

typedef enum {
    PHASE_PENDING,
    PHASE_RUNNING,
    PHASE_COMPLETED,
    PHASE_FAILED
} PhaseStatus;

typedef struct {
    char name[50];
    RStage* stages[16];
    int stage_count;
    int priority;
    PhaseStatus status;
} RPhase;

// Phase operations
RPhase* create_phase(const char* name, int priority);            // no scheduler here
void destroy_phase(RPhase* phase);

int phase_add_task(RPhase* phase, RTask* task);   // <--- Add this
int phase_add_stage(RPhase* phase, RStage* stage);
int phase_run(RPhase* phase, RTaskScheduler* sched);            // pass scheduler here
PhaseStatus phase_status(RPhase* phase);

#endif
