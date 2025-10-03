#include "roc_phase.h"
#include "roc_phase.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h> // for Sleep

RPhase* create_phase(const char* name, int priority) {
    RPhase* phase = (RPhase*)malloc(sizeof(RPhase));
    strcpy(phase->name, name);
    phase->stage_count = 0;
    phase->priority = priority;
    phase->status = PHASE_PENDING;
    return phase;
}

void destroy_phase(RPhase* phase) {
    free(phase);
}

int phase_add_task(RPhase* phase, RTask* task) {
    if (!phase || !task) return -1;

    // Create a temporary stage for this task
    RStage* tmp_stage = create_stage(task->name, 0); // use 0 or task priority
    if (!tmp_stage) return -1;

    stage_add_item(tmp_stage, task, STAGE_ITEM_TASK); // now STAGE_ITEM_TASK exists
    phase_add_stage(phase, tmp_stage);

    return 0;
}

int phase_add_stage(RPhase* phase, RStage* stage) {
    if (phase->stage_count >= 16) return 0;
    phase->stages[phase->stage_count++] = stage;
    return 1;
}

static void* run_phase_thread(void* arg) {
    struct { RPhase* phase; RTaskScheduler* sched; }* data = arg;
    RPhase* phase = data->phase;
    RTaskScheduler* sched = data->sched;
    free(data);

    phase->status = PHASE_RUNNING;

    for (int i = 0; i < phase->stage_count; i++) {
        stage_run(phase->stages[i], sched);
        while (stage_status(phase->stages[i]) != STAGE_COMPLETED) {
            Sleep(50);
        }
    }

    phase->status = PHASE_COMPLETED;
    printf("[Phase] Phase '%s' completed\n", phase->name);
    return NULL;
}

int phase_run(RPhase* phase, RTaskScheduler* sched) {
    if (!phase || !sched) return -1;
    phase->status = PHASE_RUNNING;
    for (int i = 0; i < phase->stage_count; i++) {
        stage_run(phase->stages[i], sched);
        while (stage_status(phase->stages[i]) != STAGE_COMPLETED) {
            Sleep(50); // wait
        }
    }
    phase->status = PHASE_COMPLETED;
    return 0;
}

PhaseStatus phase_status(RPhase* phase) {
    return phase->status;
}
