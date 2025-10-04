#include "hw/roc_hw_scheduler.h"
#include <windows.h>
#include <psapi.h>
#include <stdlib.h>
#include <stdio.h>

int stats_printed;

static DWORD WINAPI hw_task_runner(LPVOID arg) {
    HWTask* task = (HWTask*)arg;
    HWScheduler* sched = task->scheduler;

    if (task->assigned_core < 0) {
        EnterCriticalSection(&sched->lock);
        int min_core = 0;
        for (int i = 1; i < sched->num_cores; i++) {
            if (sched->tasks_per_core[i] < sched->tasks_per_core[min_core])
                min_core = i;
        }
        task->assigned_core = min_core;
        sched->tasks_per_core[min_core]++;
        LeaveCriticalSection(&sched->lock);
    } else {
        EnterCriticalSection(&sched->lock);
        sched->tasks_per_core[task->assigned_core]++;
        LeaveCriticalSection(&sched->lock);
    }

    SetThreadAffinityMask(GetCurrentThread(), 1ULL << task->assigned_core);
    printf("Task starting on core %d\n", task->assigned_core);

    if (task->func)
        task->func(task);

    printf("Task finished on core %d\n", task->assigned_core);

    EnterCriticalSection(&sched->lock);
    sched->tasks_per_core[task->assigned_core]--;
    sched->completed_tasks_per_core[task->assigned_core]++;
    LeaveCriticalSection(&sched->lock);

    task->is_completed = 1;

    return 0;
}

hw_task_stats_t hw_get_task_stats(HANDLE thread) {
    hw_task_stats_t stats = {0};

    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (GetThreadTimes(thread, &creationTime, &exitTime, &kernelTime, &userTime)) {
        ULARGE_INTEGER k, u;
        k.LowPart = kernelTime.dwLowDateTime;
        k.HighPart = kernelTime.dwHighDateTime;
        u.LowPart = userTime.dwLowDateTime;
        u.HighPart = userTime.dwHighDateTime;
        stats.cpu_time_ms = (k.QuadPart + u.QuadPart) / 10000;
    }

    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        stats.memory_usage_bytes = pmc.WorkingSetSize; // rough per-task memory
    }

    return stats;
}

HWScheduler* hw_create_scheduler(int num_cores) {
    HWScheduler* sched = malloc(sizeof(HWScheduler));
    sched->num_cores = num_cores;
    sched->tasks = NULL;
    sched->task_count = 0;
    sched->tasks_per_core = calloc(num_cores, sizeof(int));
    sched->completed_tasks_per_core = calloc(num_cores, sizeof(int));
    InitializeCriticalSection(&sched->lock);
    return sched;
}

void hw_scheduler_add_task(HWScheduler* sched, HWTask* task) {
    EnterCriticalSection(&sched->lock);
    task->scheduler = sched; // link task to scheduler
    sched->tasks = realloc(sched->tasks, sizeof(HWTask*) * (sched->task_count + 1));
    sched->tasks[sched->task_count++] = task;
    LeaveCriticalSection(&sched->lock);
}

void hw_scheduler_start(HWScheduler* sched) {
    for (int i = 0; i < sched->task_count; i++) {
        sched->tasks[i]->thread_handle = CreateThread(
            NULL, 0, hw_task_runner, (LPVOID)sched->tasks[i], 0, NULL
        );
    }
}

void hw_scheduler_runasync(HWScheduler* sched) {
    for (int i = 0; i < sched->task_count; i++) {
        sched->tasks[i]->thread_handle = CreateThread(
            NULL, 0, hw_task_runner, (LPVOID)sched->tasks[i], 0, NULL
        );
    }
}

void hw_scheduler_wait(HWScheduler* sched) {
    HANDLE* handles = malloc(sizeof(HANDLE) * sched->task_count);
    for (int i = 0; i < sched->task_count; i++)
        handles[i] = sched->tasks[i]->thread_handle;

    WaitForMultipleObjects(sched->task_count, handles, TRUE, INFINITE);

    // Print per-task stats after completion
    for (int i = 0; i < sched->task_count; i++) {
        if (!sched->tasks[i]->stats_printed) {
            hw_task_stats_t stats = hw_get_task_stats(sched->tasks[i]->thread_handle);
            printf("Task on core %d: CPU time = %llu ms, Memory = %llu bytes\n",
                   sched->tasks[i]->assigned_core,
                   stats.cpu_time_ms,
                   stats.memory_usage_bytes);

            sched->tasks[i]->stats_printed = 1;
        }
        CloseHandle(sched->tasks[i]->thread_handle);
    }

    printf("\nTask assignment per core:\n");
    for (int i = 0; i < sched->num_cores; i++) {
        printf("Core %d handled approx %d tasks\n",
               i,
               sched->completed_tasks_per_core[i]);
    }

    free(handles);
}

void hw_scheduler_destroy(HWScheduler* sched) {
    DeleteCriticalSection(&sched->lock);
    free(sched->tasks);
    free(sched->tasks_per_core);
    free(sched->completed_tasks_per_core);
    free(sched);
}
