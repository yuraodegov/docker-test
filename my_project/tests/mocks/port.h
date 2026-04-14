#pragma once
#include <stdint.h>
static uint8_t mock_inputs[4] = {1, 1, 1, 1};
static inline uint8_t getInput(uint8_t pin) { return mock_inputs[pin]; }
