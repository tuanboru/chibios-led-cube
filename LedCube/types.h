/*
 * types.h
 *
 *  Created on: 05/mag/2013
 *      Author: Luca
 */

#ifndef TYPES_H_
#define TYPES_H_

#include "chtypes.h"
#include "settings.h"

typedef struct {
	uint32_t layer;
} frame_t;

typedef struct {
	uint16_t delay;
	uint8_t frame_number;
	uint8_t frames[][CUBE_SIZE];
} animation_t;

typedef struct {
	void (*func)(frame_t *buffer, uint16_t arg);
	uint16_t frames;
	uint16_t delay;
} anim_func_t;

typedef struct{
	uint8_t x;
	uint8_t y;
	uint8_t z;
} point_t;

#endif /* TYPES_H_ */
