#pragma once
#include <stdint.h>
static uint32_t mock_time_ms = 0;
static inline void timerStart(uint32_t* t) { *t = mock_time_ms; }
static inline int timerTimeOut(uint32_t* t, uint32_t period) { return (mock_time_ms - *t >= period) ? 1 : 0; }
#define TIMER_TIMEOUT 1
