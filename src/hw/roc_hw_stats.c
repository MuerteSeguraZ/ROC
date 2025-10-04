#include "hw/roc_hw_stats.h"
#include <windows.h>
#include <psapi.h>

hw_system_info_t hw_get_system_info(void) {
    hw_system_info_t info = {0};

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    info.num_cores = sysinfo.dwNumberOfProcessors;
    info.num_threads = sysinfo.dwNumberOfProcessors; // simplifying

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    GlobalMemoryStatusEx(&memStatus);
    info.total_memory = memStatus.ullTotalPhys;

    return info;
}
