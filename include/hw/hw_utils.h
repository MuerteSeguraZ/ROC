#pragma once
#include <windows.h>
#include <stdint.h>
#include "roc_hw_scheduler.h"

// Time utilities
uint64_t hw_get_time_ms(void);
void hw_sleep_ms(uint32_t ms);
int hw_quantum_remaining(uint64_t start_time_ms, uint32_t quantum_ms);

// Time-slice utilities
void hw_timeslice_init(HWTask* task, uint32_t quantum_ms);
int hw_on_timeslice(HWTask* task);
void hw_yield_task(HWTask* task);
