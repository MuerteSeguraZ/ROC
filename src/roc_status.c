#include "roc_status.h"

// --- Task ---
const char* task_status_str(TaskStatus status) {
    switch(status) {
        case TASK_PENDING:   return "PENDING";
        case TASK_RUNNING:   return "RUNNING";
        case TASK_COMPLETED: return "COMPLETED";
        case TASK_FAILED:    return "FAILED";
        default:             return "UNKNOWN";
    }
}

// --- Bundle ---
const char* bundle_status_str(BundleStatus status) {
    switch(status) {
        case BUNDLE_PENDING:   return "PENDING";
        case BUNDLE_RUNNING:   return "RUNNING";
        case BUNDLE_COMPLETED: return "COMPLETED";
        case BUNDLE_FAILED:    return "FAILED";
        default:               return "UNKNOWN";
    }
}

// --- Campaign ---
const char* campaign_status_str(CampaignStatus status) {
    switch(status) {
        case CAMPAIGN_PENDING:   return "PENDING";
        case CAMPAIGN_RUNNING:   return "RUNNING";
        case CAMPAIGN_COMPLETED: return "COMPLETED";
        case CAMPAIGN_FAILED:    return "FAILED";
        default:                 return "UNKNOWN";
    }
}

// --- Program ---
const char* program_status_str(ProgramStatus status) {
    switch(status) {
        case PROGRAM_PENDING:   return "PENDING";
        case PROGRAM_RUNNING:   return "RUNNING";
        case PROGRAM_COMPLETED: return "COMPLETED";
        case PROGRAM_FAILED:    return "FAILED";
        default:                return "UNKNOWN";
    }
}
