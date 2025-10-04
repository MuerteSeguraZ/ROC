#ifndef ROC_PROGRAM_H
#define ROC_PROGRAM_H

#include "roc_campaign.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_CAMPAIGNS_PER_PROGRAM 32

typedef enum {
    PROGRAM_PENDING,
    PROGRAM_RUNNING,
    PROGRAM_COMPLETED,
    PROGRAM_FAILED
} ProgramStatus;

typedef struct {
    char name[64];
    RCampaign* campaigns[MAX_CAMPAIGNS_PER_PROGRAM];
    int campaign_count;

    int priority; // Program-level priority
    ProgramStatus status;

    pthread_mutex_t lock;
} RProgram;

// Program operations
RProgram* create_program(const char* name, int priority);
void destroy_program(RProgram* program);

int program_add_campaign(RProgram* program, RCampaign* campaign);
int program_run(RProgram* program, RTaskScheduler* sched);
ProgramStatus program_status(RProgram* program);

#endif
