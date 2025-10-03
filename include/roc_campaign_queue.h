#ifndef ROC_CAMPAIGN_QUEUE_H
#define ROC_CAMPAIGN_QUEUE_H

#include "roc_campaign.h"
#include "roc_scheduler.h"
#include <pthread.h>

#define MAX_CAMPAIGNS 32

typedef struct {
    RCampaign* campaigns[MAX_CAMPAIGNS];
    int campaign_count;
    pthread_mutex_t lock;
} RCampaignQueue;

// Queue operations
RCampaignQueue* create_campaign_queue();
void destroy_campaign_queue(RCampaignQueue* queue);

int enqueue_campaign(RCampaignQueue* queue, RCampaign* campaign);
RCampaign* dequeue_campaign(RCampaignQueue* queue);

int process_campaign_queue(RCampaignQueue* queue, RTaskScheduler* sched);

#endif
