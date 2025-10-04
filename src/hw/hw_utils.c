#include "hw/hw_utils.h"
#include <stdio.h>

// High-resolution time
uint64_t hw_get_time_ms(void) {
    static LARGE_INTEGER freq;
    static int initialized = 0;
    if (!initialized) {
        QueryPerformanceFrequency(&freq);
        initialized = 1;
    }
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t)((now.QuadPart * 1000) / freq.QuadPart);
}

// Sleep helper
void hw_sleep_ms(uint32_t ms) {
    Sleep(ms);
}

// Compute remaining quantum
int hw_quantum_remaining(uint64_t start_time_ms, uint32_t quantum_ms) {
    uint64_t now = hw_get_time_ms();
    if (now < start_time_ms) return (int)quantum_ms; // wraparound safeguard
    int elapsed = (int)(now - start_time_ms);
    return (elapsed < (int)quantum_ms) ? (int)(quantum_ms - elapsed) : 0;
}

// -------------------- Time-slice functions --------------------

// Initialize task time slice
void hw_timeslice_init(HWTask* task, uint32_t quantum_ms) {
    task->quantum_ms = quantum_ms;
    task->last_start_time = hw_get_time_ms();
}

// Check if task quantum expired
int hw_on_timeslice(HWTask* task) {
    if (!task) return 0;
    int remaining = hw_quantum_remaining(task->last_start_time, task->quantum_ms);
    return remaining > 0;
}

void hw_yield_task(HWTask* task) {
    if (!task) return;
    task->yield_requested = 1;
    Sleep(0); // yield to threads of same priority
}

