#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "button.h"
#include "dw1000_ch.h"
#include "test_app2.h"
#include "test_app.h"
#include "clock.h"
#include "consensus.h"
#include "synchro.h"
#include "board.h"
#include "sd_protocol.h"

#include "source_seeking_app.h"
#include "debug_listener.h"

#define DEBUG_ADDR		2177

uint8_t switch_app_toggle = 0;
uint8_t app_toggle = 0;

// Clock variables
int32_t clock_time = 0;
int32_t control = 0;

int32_t cont = 0;

void switch_app(void* args)
{
	switch_app_toggle = 1;
}

static THD_WORKING_AREA(WA_CLOCK, 256);
static THD_WORKING_AREA(WA_CONSENSUS, 512);

int main(void) {
	halInit();
    chSysInit();

    leds_off(ALL_LEDS);

	palEnablePadEvent(IOPORT1, DW_IRQ, PAL_EVENT_MODE_RISING_EDGE);
	palSetPadCallback(IOPORT1, DW_IRQ, ISR_wrapper, NULL);

	chThdSleepMilliseconds(100);
	chThdCreateStatic(DW_IRQ_THREAD, sizeof(DW_IRQ_THREAD), NORMALPRIO+2, DW_IRQ_HANDLER, NULL);

	chThdSleepMilliseconds(100);
	
	chThdCreateStatic(DW_CONTROLLER_THREAD, sizeof(DW_CONTROLLER_THREAD), NORMALPRIO, DW_CONTROLLER, NULL);
	chThdCreateStatic(UART_RECEIVER_THREAD, sizeof(UART_RECEIVER_THREAD), NORMALPRIO, UART_RECEIVER, NULL);
	chThdCreateStatic(UART_SENDER_THREAD, sizeof(UART_SENDER_THREAD), NORMALPRIO, UART_SENDER, NULL);
	chThdCreateStatic(DEBUG_PRINTER_THREAD, sizeof(DEBUG_PRINTER_THREAD), NORMALPRIO, DEBUG_PRINTER, NULL);
	
	chThdCreateStatic(WA_CLOCK, sizeof(WA_CLOCK),NORMALPRIO + 1, CLOCK, NULL);
	chThdCreateStatic(WA_CONSENSUS, sizeof(WA_CONSENSUS),NORMALPRIO, CONSENSUS, NULL);


	chThdSleepMilliseconds(100); 

	//if (dw_get_addr() == DEBUG_ADDR)
	//	app_thread_p = chThdCreateStatic(APP_THREAD, sizeof(APP_THREAD), NORMALPRIO, DEBUG_LISTNR, NULL);
	//else
	//	app_thread_p = chThdCreateStatic(APP_THREAD, sizeof(APP_THREAD), NORMALPRIO, SS, NULL);

	BaseSequentialStream* const chp = (BaseSequentialStream*)&SD1;

	while (true) {	
		toggle_led(red1);
		chThdSleepMilliseconds(500);

	}
}