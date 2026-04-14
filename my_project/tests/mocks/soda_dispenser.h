#pragma once
#include <stdint.h>
typedef enum { DISPENSER_READY=0, DISPENSER_FILL_WATER_FULL, DISPENSER_SODA_DISPENSE } DispenserState;
static DispenserState mock_dispenser_state = DISPENSER_READY;
static int mock_carbonate_called=0, mock_dispense_called=0, mock_stop_called=0;
static inline DispenserState getDispenserState() { return mock_dispenser_state; }
static inline void SodaCarbonate()               { mock_carbonate_called++; }
static inline void SodaDispenseMs(uint32_t ms)   { mock_dispense_called++; }
static inline void stopSodaDispense()            { mock_stop_called++; }
