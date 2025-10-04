#ifndef ROC_HW_TASK_H
#define ROC_HW_TASK_H

#include <stdint.h>

// Forward declare scheduler
struct HWScheduler;

typedef struct HWTask {
    char* name;                     // Task name
    int assigned_core;               // -1 for any core
    void (*func)(struct HWTask*);    // Function to execute
    void* user_data;                 // Optional data for the task
    int is_completed;                // 0 = running, 1 = done
} HWTask;

// Create a new hardware task
HWTask* hw_create_task(const char* name, void (*func)(HWTask*));

// Destroy a hardware task
void hw_destroy_task(HWTask* task);

#endif // ROC_HW_TASK_H
