#ifndef ROC_CAMPAIGN_H
#define ROC_CAMPAIGN_H

#include "roc_bundle.h"
#include "roc_bundle_queue.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_BUNDLES_PER_CAMPAIGN 16

typedef enum {
    CAMPAIGN_PENDING,
    CAMPAIGN_RUNNING,
    CAMPAIGN_COMPLETED,
    CAMPAIGN_FAILED
} CampaignStatus;

typedef struct {
    char name[50];
    RBundle* bundles[MAX_BUNDLES_PER_CAMPAIGN];
    int bundle_count;

    int priority;  // campaign-level priority
    CampaignStatus status;

    pthread_mutex_t lock;
} RCampaign;

// Campaign operations
RCampaign* create_campaign(const char* name, int priority);
void destroy_campaign(RCampaign* campaign);

int campaign_add_bundle(RCampaign* campaign, RBundle* bundle);
int campaign_run(RCampaign* campaign, RTaskScheduler* sched);
CampaignStatus campaign_status(RCampaign* campaign);

#endif
