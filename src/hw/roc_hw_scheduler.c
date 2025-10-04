#include "hw/roc_hw_scheduler.h"
#include <hw/hw_utils.h>
#include <windows.h>
#include <psapi.h>
#include <stdlib.h>
#include <stdio.h>

int stats_printed;

static void hw_query_topology(HWScheduler* sched) {
    DWORD len = 0;
    GetLogicalProcessorInformationEx(RelationProcessorCore, NULL, &len);
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* buffer =
        (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)malloc(len);
    if (!GetLogicalProcessorInformationEx(RelationProcessorCore, buffer, &len)) {
        sched->num_physical_cores = sched->num_logical_cores;
        sched->logical_to_physical = calloc(sched->num_logical_cores, sizeof(int));
        for (int i = 0; i < sched->num_logical_cores; i++)
            sched->logical_to_physical[i] = i;
        return;
    }

    int phys_index = 0;
    sched->logical_to_physical = calloc(sched->num_logical_cores, sizeof(int));

    BYTE* ptr = (BYTE*)buffer;
    while (ptr < (BYTE*)buffer + len) {
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX* info =
            (SYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX*)ptr;
        if (info->Relationship == RelationProcessorCore) {
            KAFFINITY mask = info->Processor.GroupMask[0].Mask;
            for (int i = 0; i < sched->num_logical_cores; i++) {
                if (mask & (1ULL << i))
                    sched->logical_to_physical[i] = phys_index;
            }
            phys_index++;
        }
        ptr += info->Size;
    }
    sched->num_physical_cores = phys_index;
    free(buffer);
}

static DWORD WINAPI hw_task_runner(LPVOID arg) {
    HWTask* task = (HWTask*)arg;
    HWScheduler* sched = task->scheduler;

    // Assign core if not already assigned
    if (task->assigned_core < 0) {
        EnterCriticalSection(&sched->lock);
        int min_core = -1;

        // Prefer spreading across physical cores first
        for (int i = 0; i < sched->num_logical_cores; i++) {
            int phys = sched->logical_to_physical[i];
            int already_used = 0;
            for (int j = 0; j < sched->num_logical_cores; j++) {
                if (sched->logical_to_physical[j] == phys &&
                    sched->tasks_per_core[j] > 0) {
                    already_used = 1;
                    break;
                }
            }
            if (!already_used) {
                min_core = i;
                break;
            }
        }

        // If all physical cores busy, pick least loaded logical
        if (min_core == -1) {
            min_core = 0;
            for (int i = 1; i < sched->num_logical_cores; i++) {
                if (sched->tasks_per_core[i] < sched->tasks_per_core[min_core])
                    min_core = i;
            }
        }

        task->assigned_core = min_core;
        sched->tasks_per_core[min_core]++;
        LeaveCriticalSection(&sched->lock);
    }

    // Bind thread to assigned logical core
    SetThreadAffinityMask(GetCurrentThread(), 1ULL << task->assigned_core);

    // Map task priority → Windows thread priority
    int prio = task->priority;
    int winprio = THREAD_PRIORITY_NORMAL;
    if (prio >= 10) winprio = THREAD_PRIORITY_HIGHEST;
    else if (prio > 0) winprio = THREAD_PRIORITY_ABOVE_NORMAL;
    else if (prio < 0 && prio > -10) winprio = THREAD_PRIORITY_BELOW_NORMAL;
    else if (prio <= -10) winprio = THREAD_PRIORITY_LOWEST;

    SetThreadPriority(GetCurrentThread(), winprio);

    printf("Task starting on core %d (priority=%d, quantum=%llu ms)\n",
           task->assigned_core, task->priority, task->quantum_ms);

    // Initialize quantum
    hw_timeslice_init(task, task->quantum_ms);

    if (task->func) {
        // Run until task is completed
        while (!task->is_completed) {
            uint64_t quantum_end = hw_get_time_ms() + task->quantum_ms;

            // Execute task until quantum expires or yield/migration occurs
            while (!task->is_completed && hw_get_time_ms() < quantum_end) {
                task->func(task);

                // Voluntary yield
                if (task->yield_requested) {
                    task->yield_requested = 0;
                    break;
                }

                // Task migration
                if (task->target_core >= 0 && task->target_core != task->assigned_core) {
                    EnterCriticalSection(&sched->lock);
                    sched->tasks_per_core[task->assigned_core]--;
                    task->assigned_core = task->target_core;
                    sched->tasks_per_core[task->assigned_core]++;
                    task->target_core = -1;
                    SetThreadAffinityMask(GetCurrentThread(), 1ULL << task->assigned_core);
                    LeaveCriticalSection(&sched->lock);
                    printf(" -> Task migrated to core %d\n", task->assigned_core);
                    break; // exit inner loop to re-evaluate
                }

                Sleep(0); // yield CPU
            }

            // Reset quantum for next round if task still running
            if (!task->is_completed) {
                hw_timeslice_init(task, task->quantum_ms);
            }
        }
    }

    printf("Task finished on core %d (priority=%d)\n",
           task->assigned_core, task->priority);

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

    SYSTEM_INFO si;
    GetSystemInfo(&si);
    sched->num_logical_cores = si.dwNumberOfProcessors;
    sched->num_physical_cores = 0;
    sched->tasks = NULL;
    sched->task_count = 0;

    hw_query_topology(sched);

    sched->tasks_per_core = calloc(sched->num_logical_cores, sizeof(int));
    sched->completed_tasks_per_core = calloc(sched->num_logical_cores, sizeof(int));
    InitializeCriticalSection(&sched->lock);
    return sched;
}

void hw_scheduler_add_task(HWScheduler* sched, HWTask* task) {
    EnterCriticalSection(&sched->lock);
    task->scheduler = sched;

    // Expand task list
    sched->tasks = realloc(sched->tasks, sizeof(HWTask*) * (sched->task_count + 1));
    sched->tasks[sched->task_count++] = task;

    // Sort tasks by priority (descending)
    for (int i = sched->task_count - 1; i > 0; i--) {
        if (sched->tasks[i]->priority > sched->tasks[i-1]->priority) {
            HWTask* tmp = sched->tasks[i];
            sched->tasks[i] = sched->tasks[i-1];
            sched->tasks[i-1] = tmp;
        }
    }

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
            printf("Task on logical core %d (physical %d): CPU time = %llu ms, Memory = %llu bytes\n",
                sched->tasks[i]->assigned_core,
                sched->logical_to_physical[sched->tasks[i]->assigned_core],
                stats.cpu_time_ms,
                stats.memory_usage_bytes);

            sched->tasks[i]->stats_printed = 1;
        }
        CloseHandle(sched->tasks[i]->thread_handle);
    }

    printf("\nTask assignment per core:\n");
    for (int i = 0; i < sched->num_logical_cores; i++) {
        printf("Logical core %d (physical %d) handled approx %d tasks\n",
               i,
               sched->logical_to_physical[i],
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
