#include "roc_program_queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Create program queue
RProgramQueue* create_program_queue(const char* name) {
    RProgramQueue* q = (RProgramQueue*)malloc(sizeof(RProgramQueue));
    if (!q) return NULL;
    strncpy(q->name, name, sizeof(q->name));
    q->program_count = 0;
    pthread_mutex_init(&q->lock, NULL);
    return q;
}

// Destroy program queue
void destroy_program_queue(RProgramQueue* queue) {
    if (!queue) return;
    pthread_mutex_destroy(&queue->lock);
    free(queue);
}

// Add program
int program_queue_add(RProgramQueue* queue, RProgram* prog) {
    if (!queue || !prog) return -1;
    pthread_mutex_lock(&queue->lock);
    if (queue->program_count >= 32) {
        pthread_mutex_unlock(&queue->lock);
        return -1;
    }
    queue->programs[queue->program_count++] = prog;
    pthread_mutex_unlock(&queue->lock);
    return 0;
}

// Run queue sequentially
int program_queue_run(RProgramQueue* queue, RTaskScheduler* sched) {
    if (!queue || !sched) return -1;
    printf("=== Running programs in queue '%s' ===\n", queue->name);

    for (int i = 0; i < queue->program_count; i++) {
        RProgram* prog = queue->programs[i];
        if (prog) {
            program_run(prog, sched);

            // Wait until program finishes
            while (program_status(prog) == PROGRAM_RUNNING) {
                usleep(50000);
            }
            printf("[ProgramQueue] Program '%s' finished with status %d\n",
                   prog->name, program_status(prog));
        }
    }

    printf("[ProgramQueue] All programs in queue '%s' completed\n", queue->name);
    return 0;
}

// Run queue by priority
int program_queue_run_priority(RProgramQueue* queue, RTaskScheduler* sched) {
    if (!queue || !sched) return -1;
    printf("=== Running programs in queue '%s' (priority order) ===\n", queue->name);

    // Simple bubble sort by priority
    for (int i = 0; i < queue->program_count - 1; i++) {
        for (int j = i + 1; j < queue->program_count; j++) {
            if (queue->programs[j]->priority > queue->programs[i]->priority) {
                RProgram* tmp = queue->programs[i];
                queue->programs[i] = queue->programs[j];
                queue->programs[j] = tmp;
            }
        }
    }

    return program_queue_run(queue, sched);
}
