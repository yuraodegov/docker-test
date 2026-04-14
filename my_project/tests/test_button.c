#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "mocks/port.h"
#include "mocks/timer.h"
#include "mocks/soda_dispenser.h"
#include "mocks/soda_level.h"
#include "mocks/ws2812b.h"
#include "mocks/terminal.h"
#include "mocks/button.h"
#include "mocks/input_filter.h"
#include "../src/button.c"

static void reset_all(void) {
    mock_carbonate_called=0; mock_dispense_called=0;
    mock_stop_called=0; mock_level_update_called=0;
    mock_dispenser_state=DISPENSER_READY;
    mock_time_ms=0;
    memset(mock_inputs,1,sizeof(mock_inputs));
}

static void press_button(uint8_t btn, uint32_t hold_ms) {
    mock_time_ms += BUTTON_WAIT_FOR_RLEASE+1; buttonHandler();
    mock_inputs[btn]=0; buttonHandler();
    mock_time_ms += hold_ms; buttonHandler();
    mock_inputs[btn]=1; buttonHandler();
}

static void test_init_state(void) {
    reset_all(); buttonsInitialize();
    for(int i=0;i<TOTAL_INPUTS;i++) assert(push_button[i].state==WAIT_FOR_RELEASE);
    printf("[PASS] test_init_state\n");
}
static void test_button2_ready(void) {
    reset_all(); buttonsInitialize();
    mock_dispenser_state=DISPENSER_READY;
    press_button(BUTTON_2,50);
    assert(mock_dispense_called==1); assert(mock_stop_called==0);
    printf("[PASS] test_button2_ready\n");
}
static void test_button2_not_ready(void) {
    reset_all(); buttonsInitialize();
    mock_dispenser_state=DISPENSER_SODA_DISPENSE;
    press_button(BUTTON_2,50);
    assert(mock_stop_called==1); assert(mock_dispense_called==0);
    printf("[PASS] test_button2_not_ready\n");
}
static void test_button1_carbonate(void) {
    reset_all(); buttonsInitialize();
    mock_dispenser_state=DISPENSER_FILL_WATER_FULL;
    press_button(BUTTON_1,50);
    assert(mock_carbonate_called==1);
    printf("[PASS] test_button1_carbonate\n");
}
static void test_button1_no_carbonate(void) {
    reset_all(); buttonsInitialize();
    mock_dispenser_state=DISPENSER_READY;
    press_button(BUTTON_1,50);
    assert(mock_carbonate_called==0);
    printf("[PASS] test_button1_no_carbonate\n");
}
static void test_button3_level(void) {
    reset_all(); buttonsInitialize();
    press_button(BUTTON_3,50);
    assert(mock_level_update_called==1);
    printf("[PASS] test_button3_level\n");
}
static void test_long_press(void) {
    reset_all(); buttonsInitialize();
    mock_time_ms+=BUTTON_WAIT_FOR_RLEASE+1; buttonHandler();
    mock_inputs[BUTTON_1]=0; buttonHandler();
    mock_time_ms+=BUTTON_LONG_CLICK_PERIOD+1; buttonHandler();
    assert(push_button[BUTTON_1].state==LONG_CLICK);
    printf("[PASS] test_long_press\n");
}
static void test_double_click(void) {
    reset_all(); buttonsInitialize();
    mock_time_ms+=BUTTON_WAIT_FOR_RLEASE+1; buttonHandler();
    mock_inputs[BUTTON_2]=0; buttonHandler();
    mock_inputs[BUTTON_2]=1; buttonHandler();
    mock_time_ms+=BUTTON_DOUBLE_CLICK_EXPIRED/2;
    mock_inputs[BUTTON_2]=0; buttonHandler();
    assert(push_button[BUTTON_2].state==WAIT_FOR_RELEASE);
    printf("[PASS] test_double_click\n");
}

int main(void) {
    printf("======= Button Tests =======\n");
    test_init_state();
    test_button2_ready();
    test_button2_not_ready();
    test_button1_carbonate();
    test_button1_no_carbonate();
    test_button3_level();
    test_long_press();
    test_double_click();
    printf("============================\n");
    printf("All tests passed!\n");
    return 0;
}
