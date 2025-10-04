#include "hw/roc_hw_scheduler.h"
#include <windows.h>
#include <psapi.h>
#include <stdlib.h>
#include <stdio.h>

int stats_printed;

static DWORD WINAPI hw_task_runner(LPVOID arg) {
    HWTask* task = (HWTask*)arg;

    // Pin thread to assigned core
    if (task->assigned_core >= 0) {
        DWORD_PTR mask = 1ULL << task->assigned_core;
        SetThreadAffinityMask(GetCurrentThread(), mask);
    }

    // Run the actual task function
    if (task->func)
        task->func(task);

    task->is_completed = 1; // Signal completion
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
    InitializeCriticalSection(&sched->lock);
    return sched;
}

void hw_scheduler_add_task(HWScheduler* sched, HWTask* task) {
    EnterCriticalSection(&sched->lock);
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

        sched->tasks[i]->stats_printed = 1; // mark as printed
    }
    CloseHandle(sched->tasks[i]->thread_handle);
  }
    free(handles);
}

void hw_scheduler_destroy(HWScheduler* sched) {
    DeleteCriticalSection(&sched->lock);
    free(sched->tasks);
    free(sched);
}
