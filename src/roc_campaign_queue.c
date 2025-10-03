#include "roc_campaign_queue.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

RCampaignQueue* create_campaign_queue() {
    RCampaignQueue* queue = (RCampaignQueue*)malloc(sizeof(RCampaignQueue));
    queue->campaign_count = 0;
    pthread_mutex_init(&queue->lock, NULL);
    return queue;
}

void destroy_campaign_queue(RCampaignQueue* queue) {
    pthread_mutex_destroy(&queue->lock);
    free(queue);
}

// Insert in priority order (higher priority first)
int enqueue_campaign(RCampaignQueue* queue, RCampaign* campaign) {
    pthread_mutex_lock(&queue->lock);
    if (queue->campaign_count >= MAX_CAMPAIGNS) {
        pthread_mutex_unlock(&queue->lock);
        return 0;
    }

    int i = queue->campaign_count - 1;
    while (i >= 0 && queue->campaigns[i]->priority < campaign->priority) {
        queue->campaigns[i + 1] = queue->campaigns[i];
        i--;
    }
    queue->campaigns[i + 1] = campaign;
    queue->campaign_count++;
    pthread_mutex_unlock(&queue->lock);
    return 1;
}

RCampaign* dequeue_campaign(RCampaignQueue* queue) {
    pthread_mutex_lock(&queue->lock);
    if (queue->campaign_count == 0) {
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }

    RCampaign* campaign = queue->campaigns[0];
    for (int i = 1; i < queue->campaign_count; i++) {
        queue->campaigns[i - 1] = queue->campaigns[i];
    }
    queue->campaign_count--;
    pthread_mutex_unlock(&queue->lock);
    return campaign;
}

int process_campaign_queue(RCampaignQueue* queue, RTaskScheduler* sched) {
    while (1) {
        RCampaign* campaign = dequeue_campaign(queue);
        if (!campaign) break;

        campaign_run(campaign, sched);

        // Wait for completion
        while (campaign_status(campaign) != CAMPAIGN_COMPLETED &&
               campaign_status(campaign) != CAMPAIGN_FAILED) {
            usleep(50000);
        }
        printf("[CampaignQueue] Campaign '%s' finished with status %d\n",
               campaign->name, campaign_status(campaign));
    }
    return 1;
}
