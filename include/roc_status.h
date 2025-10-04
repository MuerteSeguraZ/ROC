#ifndef ROC_STATUS_H
#define ROC_STATUS_H

#include "roc_task.h"
#include "roc_bundle.h"
#include "roc_campaign.h"
#include "roc_program.h"

const char* task_status_str(TaskStatus status);

const char* bundle_status_str(BundleStatus status);

const char* campaign_status_str(CampaignStatus status);

const char* program_status_str(ProgramStatus status);

#endif
