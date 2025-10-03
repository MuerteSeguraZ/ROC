#include "roc_campaign.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

RCampaign* create_campaign(const char* name, int priority) {
    RCampaign* campaign = (RCampaign*)malloc(sizeof(RCampaign));
    strcpy(campaign->name, name);
    campaign->bundle_count = 0;
    campaign->priority = priority;
    campaign->status = CAMPAIGN_PENDING;
    pthread_mutex_init(&campaign->lock, NULL);
    return campaign;
}

void destroy_campaign(RCampaign* campaign) {
    pthread_mutex_destroy(&campaign->lock);
    free(campaign);
}

int campaign_add_bundle(RCampaign* campaign, RBundle* bundle) {
    pthread_mutex_lock(&campaign->lock);
    if (campaign->bundle_count >= MAX_BUNDLES_PER_CAMPAIGN) {
        pthread_mutex_unlock(&campaign->lock);
        return 0;
    }
    campaign->bundles[campaign->bundle_count++] = bundle;
    pthread_mutex_unlock(&campaign->lock);
    return 1;
}

// Helper to update campaign status
static void update_campaign_status(RCampaign* campaign) {
    pthread_mutex_lock(&campaign->lock);
    int all_completed = 1;
    int any_failed = 0;

    for (int i = 0; i < campaign->bundle_count; i++) {
        BundleStatus s = bundle_status(campaign->bundles[i]);
        if (s != BUNDLE_COMPLETED) all_completed = 0;
        if (s == BUNDLE_FAILED) any_failed = 1;
    }

    if (any_failed) campaign->status = CAMPAIGN_FAILED;
    else if (all_completed) campaign->status = CAMPAIGN_COMPLETED;
    else campaign->status = CAMPAIGN_RUNNING;

    pthread_mutex_unlock(&campaign->lock);
}

int campaign_run(RCampaign* campaign, RTaskScheduler* sched) {
    pthread_mutex_lock(&campaign->lock);
    if (campaign->bundle_count == 0) {
        pthread_mutex_unlock(&campaign->lock);
        return 0;
    }
    campaign->status = CAMPAIGN_RUNNING;
    pthread_mutex_unlock(&campaign->lock);

    for (int i = 0; i < campaign->bundle_count; i++) {
        bundle_run(campaign->bundles[i], sched);

        // Wait for bundle completion
        while (bundle_status(campaign->bundles[i]) != BUNDLE_COMPLETED &&
               bundle_status(campaign->bundles[i]) != BUNDLE_FAILED) {
            usleep(50000);
        }
        printf("[Campaign] Bundle '%s' finished with status %d\n",
               campaign->bundles[i]->name, bundle_status(campaign->bundles[i]));
    }
    return 1;
}

CampaignStatus campaign_status(RCampaign* campaign) {
    update_campaign_status(campaign);
    pthread_mutex_lock(&campaign->lock);
    CampaignStatus s = campaign->status;
    pthread_mutex_unlock(&campaign->lock);
    return s;
}
