/*
 *  Status LED driver and error reporting
 *  Yellow color LED (for some reason)
 */

#include "status.h"
#include <xc.h>
#include <stdint.h>          /* For uint32_t definition */
#include <stdbool.h>         /* For true/false definition */
// #include <p24Fxxxx.h>
#include "config.h"

// StatusLED constants
#define BLINK_BASE             1600
#define BLINK_ZERO             50
#define BLINK_ONE              300
#define BLINK_PAUSE_AFTER_ZERO 200
#define BLINK_PAUSE_AFTER_ONE  600

void StatusWait (unsigned time) {
	unsigned i;
	while (time--) {
		i = BLINK_BASE;
		while (i--) { }
	}
}

void StatusLED (unsigned status) {
	if (status) {
		LATCbits.LATC4 = 0; // on
	} else {
		LATCbits.LATC4 = 1; // off
	}
}

void StatusShow (unsigned message, unsigned length) {
	StatusLED (0);
	StatusWait (BLINK_PAUSE_AFTER_ONE);
	while (length--) {
		StatusLED (1);
		if (message & 1) {
			StatusWait (BLINK_ONE);
			StatusLED (0);
			StatusWait (BLINK_PAUSE_AFTER_ONE);
		} else {
			StatusWait (BLINK_ZERO);
			StatusLED (0);
			StatusWait (BLINK_PAUSE_AFTER_ZERO);
		}
		message = message >> 1;
	}
}

