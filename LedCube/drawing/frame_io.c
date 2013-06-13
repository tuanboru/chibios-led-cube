/*
 * frame_io.c
 *
 *  Created on: 05/mag/2013
 *      Author: Luca
 */

#include "ch.h"
#include "hal.h"

#include "frame_io.h"
#include "bitfields.h"

/*
 * This select 0..24 leds given 4 latches
 */
#define LATCHES	4

void select_led(uint32_t leds) {
	uint32_t buffer = leds;
	uint8_t i = 0;
	uint8_t high = 0;

	spiSelect(&SPID2); /* Slave Select assertion.          */
	for (i = LATCHES; i != 0; i--) {
		high = (buffer >> ((i - 1) * 8)) & 0xFF;
		spiSend(&SPID2, 1, &high);
	}
	spiUnselect(&SPID2); /* Slave Select de-assertion.       */

}
