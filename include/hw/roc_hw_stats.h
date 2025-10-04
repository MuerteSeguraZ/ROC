#pragma once
#include <windows.h>
#include <stdint.h>

typedef struct {
    uint64_t cpu_time_ms;
    SIZE_T memory_usage_bytes;
} hw_task_stats_t;

typedef struct {
    int num_cores;
    int num_threads;
    SIZE_T total_memory;
} hw_system_info_t;

// System info
hw_system_info_t hw_get_system_info(void);

// Task stats (Windows thread HANDLE)
hw_task_stats_t hw_get_task_stats(HANDLE thread);
