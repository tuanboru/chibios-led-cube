/*
 * frame_io.h
 *
 *  Created on: 05/mag/2013
 *      Author: Luca
 */

#ifndef FRAME_IO_H_
#define FRAME_IO_H_

#include "types.h"
#include "bitfields.h"

#define enable_frame(index) \
	palWriteGroup(GPIOE, PAL_GROUP_MASK(7), 2, PAL_PORT_BIT(index))

#define clear_led(buffer, x, y, z) \
	BIT_CLEAR(buffer[z].layer,BIT((x)+(y)*CUBE_SIZE))

#define set_led(buffer, x, y, z) \
	BIT_SET(buffer[z].layer,BIT((x)+(y)*CUBE_SIZE))

#define get_led(buffer, x, y, z) \
	BF_GET(buffer[z].layer, ((x)+(y*CUBE_SIZE)), 1)

#define set_frame(frame, index, pattern) \
	BF_SET((frame)->layer, pattern, index*CUBE_SIZE, CUBE_SIZE)

#define get_pattern(frame, index) \
	BF_GET(frame.layer, index*CUBE_SIZE, CUBE_SIZE*CUBE_SIZE)

#define create_frame_function(buffer, func, frame) \
	func(buffer, frame)

void create_frame(uint8_t frame[], frame_t *buffer);
void select_led(uint32_t leds);

#endif /* FRAME_IO_H_ */
