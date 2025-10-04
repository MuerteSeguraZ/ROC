#include "roc_program.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// Create a new program
RProgram* create_program(const char* name, int priority) {
    RProgram* prog = (RProgram*)malloc(sizeof(RProgram));
    if (!prog) return NULL;

    strncpy(prog->name, name, sizeof(prog->name) - 1);
    prog->name[sizeof(prog->name) - 1] = '\0';

    prog->campaign_count = 0;
    prog->priority = priority;
    prog->status = PROGRAM_PENDING;

    pthread_mutex_init(&prog->lock, NULL);
    return prog;
}

// Destroy a program
void destroy_program(RProgram* program) {
    if (!program) return;
    pthread_mutex_destroy(&program->lock);
    free(program);
}

// Add campaign to program
int program_add_campaign(RProgram* program, RCampaign* campaign) {
    if (!program || !campaign) return 0;

    pthread_mutex_lock(&program->lock);
    if (program->campaign_count >= MAX_CAMPAIGNS_PER_PROGRAM) {
        pthread_mutex_unlock(&program->lock);
        return 0;
    }

    program->campaigns[program->campaign_count++] = campaign;
    pthread_mutex_unlock(&program->lock);
    return 1;
}

// Update program status
static void update_program_status(RProgram* program) {
    pthread_mutex_lock(&program->lock);

    int all_completed = 1;
    int any_failed = 0;

    for (int i = 0; i < program->campaign_count; i++) {
        CampaignStatus cs = campaign_status(program->campaigns[i]);
        if (cs != CAMPAIGN_COMPLETED) all_completed = 0;
        if (cs == CAMPAIGN_FAILED) any_failed = 1;
    }

    if (any_failed) program->status = PROGRAM_FAILED;
    else if (all_completed) program->status = PROGRAM_COMPLETED;
    else program->status = PROGRAM_RUNNING;

    pthread_mutex_unlock(&program->lock);
}

// Run all campaigns inside the program
int program_run(RProgram* program, RTaskScheduler* sched) {
    if (!program || !sched) return 0;

    pthread_mutex_lock(&program->lock);
    if (program->campaign_count == 0) {
        pthread_mutex_unlock(&program->lock);
        return 0;
    }

    program->status = PROGRAM_RUNNING;
    pthread_mutex_unlock(&program->lock);

    printf("[Program] Starting program '%s'\n", program->name);

    for (int i = 0; i < program->campaign_count; i++) {
        campaign_run(program->campaigns[i], sched);
    }

    return 1;
}

// Get program status
ProgramStatus program_status(RProgram* program) {
    update_program_status(program);
    pthread_mutex_lock(&program->lock);
    ProgramStatus s = program->status;
    pthread_mutex_unlock(&program->lock);
    return s;
}
