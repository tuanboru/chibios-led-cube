/*
 * utils.h
 *
 *  Created on: 20/mag/2013
 *      Author: Luca
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "types.h"

#define PLANE_XY	0
#define PLANE_XZ	1
#define PLANE_YZ	2

#define AXIS_X		0
#define AXIS_Y		1
#define AXIS_Z		2

#define ROT_CENTER	(CUBE_SIZE/2)

#define map(in, inMin, inMax, outMin, outMax) \
	(((double)in - (double)inMin) / ((double)inMax - (double)inMin) * ((double)outMax - (double)outMin) + (double)outMin)

void draw_char(frame_t *buffer, uint8_t c, uint8_t axis, uint8_t plane);
void draw_plane(frame_t *buffer, uint8_t plane, uint8_t axis);
void draw_line(frame_t *buffer, point_t start, point_t end);
void rotate_axis(frame_t *buffer, int16_t angle, uint8_t axis);

#endif /* UTILS_H_ */
