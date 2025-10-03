#include "roc_phase_queue.h"
#include "roc_phase.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // for usleep

RPhaseQueue* create_phase_queue(const char* name) {
    RPhaseQueue* queue = (RPhaseQueue*)malloc(sizeof(RPhaseQueue));
    if (!queue) return NULL;
    strncpy(queue->name, name, sizeof(queue->name));
    queue->phase_count = 0;
    return queue;
}

void destroy_phase_queue(RPhaseQueue* queue) {
    if (!queue) return;
    for (int i = 0; i < queue->phase_count; i++) {
        destroy_phase(queue->phases[i]);
    }
    free(queue);
}

int phase_queue_add(RPhaseQueue* queue, RPhase* phase) {
    if (!queue || !phase || queue->phase_count >= MAX_PHASES_PER_QUEUE)
        return -1;
    queue->phases[queue->phase_count++] = phase;
    return 0;
}

int phase_queue_run(RPhaseQueue* queue, RTaskScheduler* sched) {
    if (!queue || !sched) return -1;
    printf("=== Running phases in queue '%s' ===\n", queue->name);
    for (int i = 0; i < queue->phase_count; i++) {
        printf("[PhaseQueue] Starting phase '%s'\n", queue->phases[i]->name);
        phase_run(queue->phases[i], sched);
        while (phase_status(queue->phases[i]) != PHASE_COMPLETED) {
            usleep(50000);
        }
        printf("[PhaseQueue] Phase '%s' completed\n", queue->phases[i]->name);
    }
    printf("[PhaseQueue] All phases in queue '%s' completed\n", queue->name);
    return 0;
}

int phase_queue_run_priority(RPhaseQueue* queue, RTaskScheduler* sched) {
    if (!queue || !sched) return -1;
    // simple selection sort based on phase->priority
    for (int i = 0; i < queue->phase_count - 1; i++) {
        int max_idx = i;
        for (int j = i + 1; j < queue->phase_count; j++) {
            if (queue->phases[j]->priority > queue->phases[max_idx]->priority)
                max_idx = j;
        }
        if (max_idx != i) {
            RPhase* tmp = queue->phases[i];
            queue->phases[i] = queue->phases[max_idx];
            queue->phases[max_idx] = tmp;
        }
    }
    return phase_queue_run(queue, sched);
}
