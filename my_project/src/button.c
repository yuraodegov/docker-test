#include <stdint.h>
#include "port.h"
#include "button.h"
#include "timer.h"
#include "terminal.h"
#include "soda_dispenser.h"
#include "soda_level.h"
#include "ws2812b.h"
#include "input_filter.h"

#define BUTTON_STRING(str)
#define BUTTON_PRINTF(...)

typedef enum { BUTTON_RELEASE=0, BUTTON_PRESS } ButtonPosition;
typedef enum { WAIT_FOR_RELEASE=0, WAIT_FOR_CLICK, SHORT_CLICK, SECOND_CLICK, LONG_CLICK } ButtonState;
typedef struct { ButtonState state; uint32_t timer; } ButtonSwitch;

ButtonSwitch push_button[TOTAL_INPUTS];
InputFilter  button_debouncer[TOTAL_INPUTS];

static void button_first_click(uint8_t button) {
    if (button == BUTTON_1) { if (getDispenserState()==DISPENSER_FILL_WATER_FULL) SodaCarbonate(); }
    if (button == BUTTON_2) { if (getDispenserState()==DISPENSER_READY) SodaDispenseMs(1); else stopSodaDispense(); }
    if (button == BUTTON_3) { sodaLevelUpdate(); }
}
static void button_first_release(uint8_t button) {}
static void button_long_click(uint8_t button) {}
static void button_double_click(uint8_t button) {}
static void button_long_release(uint8_t button) {}

void buttonsInitialize(void) {
    for (uint8_t i=0; i<TOTAL_INPUTS; i++) {
        initializeInputFilter(&button_debouncer[i], BUTTON_DEBOUNCE_FILTER);
        timerStart(&push_button[i].timer);
        push_button[i].state = WAIT_FOR_RELEASE;
    }
}

void buttonHandler(void) {
    for (uint8_t i=0; i<TOTAL_INPUTS; i++) {
        updateInputFilter(&button_debouncer[i], getInput(BUTTON_1+i));
        ButtonPosition position = (getInputFilter(&button_debouncer[i]) ? BUTTON_RELEASE : BUTTON_PRESS);
        switch(push_button[i].state) {
        case WAIT_FOR_RELEASE:
            if (position != BUTTON_RELEASE) { timerStart(&push_button[i].timer); break; }
            else if (timerTimeOut(&push_button[i].timer, BUTTON_WAIT_FOR_RLEASE)==TIMER_TIMEOUT) push_button[i].state++;
        case WAIT_FOR_CLICK:
            if (position==BUTTON_PRESS) { button_first_click(i); timerStart(&push_button[i].timer); push_button[i].state++; }
            break;
        case SHORT_CLICK:
            if (position!=BUTTON_PRESS) { button_first_release(i); push_button[i].state++; }
            else if (timerTimeOut(&push_button[i].timer, BUTTON_LONG_CLICK_PERIOD)==TIMER_TIMEOUT) { button_long_click(i); push_button[i].state=LONG_CLICK; }
            break;
        case SECOND_CLICK:
            if (timerTimeOut(&push_button[i].timer, BUTTON_DOUBLE_CLICK_EXPIRED)==TIMER_TIMEOUT) push_button[i].state=WAIT_FOR_CLICK;
            else if (position==BUTTON_PRESS) { button_double_click(i); timerStart(&push_button[i].timer); push_button[i].state=WAIT_FOR_RELEASE; }
            break;
        case LONG_CLICK:
            if (position==BUTTON_RELEASE) { button_long_release(i); push_button[i].state=WAIT_FOR_CLICK; }
            break;
        default:
            timerStart(&push_button[i].timer); push_button[i].state=WAIT_FOR_RELEASE;
        }
    }
}
