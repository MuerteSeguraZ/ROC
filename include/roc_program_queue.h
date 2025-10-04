#ifndef ROC_PROGRAM_QUEUE_H
#define ROC_PROGRAM_QUEUE_H

#include "roc_program.h"
#include "roc_scheduler.h"
#include <pthread.h>

typedef struct {
    char name[50];
    RProgram* programs[32];
    int program_count;
    pthread_mutex_t lock;
} RProgramQueue;

// Queue operations
RProgramQueue* create_program_queue(const char* name);
void destroy_program_queue(RProgramQueue* queue);

int program_queue_add(RProgramQueue* queue, RProgram* prog);
int program_queue_run(RProgramQueue* queue, RTaskScheduler* sched);   // sequential run
int program_queue_run_priority(RProgramQueue* queue, RTaskScheduler* sched); // run by priority

#endif
