#include "hw/roc_hw_task.h"
#include <stdlib.h>
#include <string.h>

HWTask* hw_create_task(const char* name, void (*func)(HWTask*)) {
    HWTask* task = malloc(sizeof(HWTask));
    task->name = strdup(name);
    task->assigned_core = -1;  // default: any core
    task->func = func;
    task->user_data = NULL;
    task->is_completed = 0;
    return task;
}

void hw_destroy_task(HWTask* task) {
    if (!task) return;
    free(task->name);
    free(task);
}
