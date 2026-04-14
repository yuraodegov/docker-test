#pragma once
#include <stdint.h>
#define BUTTON_1  0
#define BUTTON_2  1
#define BUTTON_3  2
#define TOTAL_INPUTS                3
#define BUTTON_DEBOUNCE_FILTER      4
#define BUTTON_WAIT_FOR_RLEASE      10
#define BUTTON_LONG_CLICK_PERIOD    1000
#define BUTTON_DOUBLE_CLICK_EXPIRED 300
void buttonsInitialize(void);
void buttonHandler(void);
