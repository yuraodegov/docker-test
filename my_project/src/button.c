/*
 * button.c
 *
 *  Created on: Aug 9, 2023
 *      Author: Ori Y.
 */

/* Includes */
#include <stdint.h>

#include "port.h"
#include "button.h"
#include "timer.h"
#include "terminal.h"
#include "soda_dispenser.h"
#include "soda_level.h"
#include "ws2812b.h"
#include "input_filter.h"


#ifdef  BUTTON_DEBUG
#define BUTTON_STRING(str)		terminalString(str)
#define BUTTON_PRINTF(...)    	terminalPrint(__VA_ARGS__)
#else
#define BUTTON_STRING(str)
#define BUTTON_PRINTF(...)
#endif /* BUTTON_DEBUG */


/* Define global variables */
typedef enum
{
	BUTTON_RELEASE = 0,
	BUTTON_PRESS
} ButtonPosition;

typedef enum
{
	WAIT_FOR_RELEASE = 0,
	WAIT_FOR_CLICK,
	SHORT_CLICK,
	SECOND_CLICK,
	LONG_CLICK
} ButtonState;


typedef struct
{
	ButtonState	state;
	uint32_t	timer;
} ButtonSwitch;


ButtonSwitch push_button[TOTAL_INPUTS];
InputFilter  button_debouncer[TOTAL_INPUTS];


/* Defines private function prototypes */

/* Button tasks */
static void button_first_click(uint8_t button);
static void button_first_release(uint8_t button);
static void button_long_click(uint8_t button);
static void button_double_click(uint8_t button);
static void button_long_release(uint8_t button);


static void button_first_click(uint8_t button)
{
	if (button == BUTTON_1)
	{
		if (getDispenserState() == DISPENSER_FILL_WATER_FULL)
				{
					SodaCarbonate();

				}

	}

	if (button == BUTTON_2)
	{
		if (getDispenserState() == DISPENSER_READY)
		{
			SodaDispenseMs(1);

		}
		else
		{
			stopSodaDispense();
		}

		//		if (getDispenserState() == DISPENSER_READY)
		//		{
		//			SodaDispenseMs(60 * 1000);
		//			setRingRotateOn(60000/13);
		//		}
		//		else
		//		{
		//			stopSodaDispense();
		//		}
	}

	if (button == BUTTON_3)
	{
		sodaLevelUpdate();
	}
}


static void button_first_release(uint8_t button)
{

}


static void button_long_click(uint8_t button)
{
	if (button == BUTTON_1)
	{
//		if (getDispenserState() == DISPENSER_SODA_DISPENSE)
//		{
//			updateSodaDispenseMs(60 * 1000);
//		}
	}
}


static void button_double_click(uint8_t button)
{

}


static void button_long_release(uint8_t button)
{
	if (button == BUTTON_1)
	{
//		if (getDispenserState() == DISPENSER_SODA_DISPENSE)
//		{
//			stopSodaDispense();
//		}
	}
}


void buttonsInitialize(void)
{
	/* Initialize push Button filter */
	for (uint8_t i = 0; i < TOTAL_INPUTS; i++)
	{
		initializeInputFilter(&button_debouncer[i], BUTTON_DEBOUNCE_FILTER);

		/* Clear all push button variables */
		timerStart(&push_button[i].timer);
		push_button[i].state = WAIT_FOR_RELEASE;
	}
}


void buttonHandler(void)
{
	for (uint8_t i = 0; i < TOTAL_INPUTS; i++)
	{
		/* Read push button position with De-bounce */
		updateInputFilter(&button_debouncer[i], getInput(BUTTON_1 + i));

		ButtonPosition position = (getInputFilter(&button_debouncer[i]) ? BUTTON_RELEASE : BUTTON_PRESS);

		/* Push Buttons states */
		switch(push_button[i].state)
		{

		case WAIT_FOR_RELEASE:

			if (position != BUTTON_RELEASE)
			{
				timerStart(&push_button[i].timer);
				break;
			}
			else if (timerTimeOut(&push_button[i].timer, BUTTON_WAIT_FOR_RLEASE) == TIMER_TIMEOUT)
			{
				push_button[i].state++;
			}


		case WAIT_FOR_CLICK:

			if (position == BUTTON_PRESS)
			{
				/* Push button first press */
				BUTTON_PRINTF("Push button No. %d First press\r\n", i);

				/* Execute button task */
				button_first_click(i);

				timerStart(&push_button[i].timer);
				push_button[i].state++;
			}
			break;


		case SHORT_CLICK:

			if (position != BUTTON_PRESS)
			{
				/* Button was released detected */
				BUTTON_PRINTF("Push button No. %d Released\r\n", i);

				/* Execute button command */
				button_first_release(i);

				/* Look for Second press */
				push_button[i].state++;
			}
			/* Button is still pressed check for long press */
			else if ((timerTimeOut(&push_button[i].timer, BUTTON_LONG_CLICK_PERIOD) == TIMER_TIMEOUT))
			{
				/* Long press detected */
				BUTTON_STRING("Push button Long press\r\n");

				/* Execute button task */
				button_long_click(i);

				/* Wait for button release */
				push_button[i].state = LONG_CLICK;
			}
			break;


		case SECOND_CLICK:

			/* If button is released check if window time for the second click is over */
			if (timerTimeOut(&push_button[i].timer, BUTTON_DOUBLE_CLICK_EXPIRED) == TIMER_TIMEOUT)
			{
				/* No Second click found */
				push_button[i].state = WAIT_FOR_CLICK;
			}
			/* Push button is pressed again within the time window */
			else if (position == BUTTON_PRESS)
			{
				/* Second press detected */
				BUTTON_PRINTF("Push button No. $d Double press\r\n", i);

				/* Execute button task */
				button_double_click(i);

				/* Wait for Button release */
				timerStart(&push_button[i].timer);
				push_button[i].state = WAIT_FOR_RELEASE;
			}
			break;


		case LONG_CLICK:

			if (position == BUTTON_RELEASE)
			{
				/* Push button released after long press */
				BUTTON_PRINTF("Push button No. %d Long press released\r\n", i);

				/* Execute button task */
				button_long_release(i);

				/* Go back and wait for button press */
				push_button[i].state = WAIT_FOR_CLICK;
			}
			break;


		default:
			timerStart(&push_button[i].timer);
			push_button[i].state = WAIT_FOR_RELEASE;
		}
	}
}

}}