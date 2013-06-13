/*
 * utils.c
 *
 *  Created on: 20/mag/2013
 *      Author: Luca
 */

#include "utils.h"
#include "types.h"
#include "settings.h"
#include "frame_io.h"
#include "math.h"
#include "string.h"
#include "chars.h"
#include "ctype.h"

void draw_char(frame_t *buffer, uint8_t c, uint8_t axis, uint8_t plane) {
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t z = 0;
	uint8_t i = 0;

	if (isalpha(c))
		i = toupper(c) - 'A';
	else if (isdigit(c))
		i = c - '0' + 26;
	else
		return;

	switch (plane) {
	case PLANE_XY:
		for (x = 0; x < CUBE_SIZE; x++)
			BF_SET(buffer[axis].layer, (chars55[i][x]>>2), x*CUBE_SIZE,
					CUBE_SIZE);
		break;
	case PLANE_XZ:
		for (z = 0; z < CUBE_HEIGHT; z++) {
			for (x = 0; x < CUBE_SIZE; x++) {
				if (BF_GET((chars55[i][z]>>2), x, 1))
					set_led(buffer, x, axis, z);
			}
		}
		break;
	case PLANE_YZ:
		for (z = 0; z < CUBE_HEIGHT; z++) {
			for (y = 0; y < CUBE_SIZE; y++) {
				if (BF_GET((chars55[i][z]>>2), (CUBE_SIZE-1)-y, 1))
					set_led(buffer, axis, y, z);
			}
		}
		break;
	}

}

void rotate_axis(frame_t *buffer, int16_t angle, uint8_t axis) {
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t z = 0;

	double theta = angle * M_PI / 180.0;

	frame_t layers[CUBE_HEIGHT];

	memset(layers, 0, sizeof(frame_t) * CUBE_HEIGHT);
	double c = round(cos(theta));
	double s = round(sin(theta));

	switch (axis) {
	case AXIS_Z:
		for (z = 0; z < CUBE_HEIGHT; z++) {
			for (y = 0; y < CUBE_SIZE; y++) {
				for (x = 0; x < CUBE_SIZE; x++) {
					if (get_led(buffer,x,y,z)) {
						double ax = c * (x - ROT_CENTER);
						double bx = s * (y - ROT_CENTER);
						double ay = s * (x - ROT_CENTER);
						double by = c * (y - ROT_CENTER);
						uint8_t xout = round(ax - bx + ROT_CENTER);
						uint8_t yout = round(ay + by + ROT_CENTER);

						if (xout <= (CUBE_SIZE - 1) && (yout <= CUBE_SIZE - 1))
							set_led(layers, xout, yout, z);
					}
				}
			}
		}
		break;
	case AXIS_Y:
		for (z = 0; z < CUBE_HEIGHT; z++) {
			for (y = 0; y < CUBE_SIZE; y++) {
				for (x = 0; x < CUBE_SIZE; x++) {
					if (get_led(buffer,x,y,z)) {
						double ax = c * (z - ROT_CENTER);
						double bx = s * (x - ROT_CENTER);
						double az = s * (z - ROT_CENTER);
						double bz = c * (x - ROT_CENTER);
						uint8_t xout = round(ax + bx + ROT_CENTER);
						uint8_t zout = round(-az + bz + ROT_CENTER);

						if (zout <= (CUBE_HEIGHT - 1)
								&& (xout <= CUBE_SIZE - 1))
							set_led(layers, xout, y, zout);
					}
				}
			}
		}
		break;
	case AXIS_X:
		for (z = 0; z < CUBE_HEIGHT; z++) {
			for (y = 0; y < CUBE_SIZE; y++) {
				for (x = 0; x < CUBE_SIZE; x++) {
					if (get_led(buffer,x,y,z)) {
						double ay = c * (z - ROT_CENTER);
						double by = s * (y - ROT_CENTER);
						double az = s * (z - ROT_CENTER);
						double bz = c * (y - ROT_CENTER);
						uint8_t yout = round(ay - by + ROT_CENTER);
						uint8_t zout = round(az + bz + ROT_CENTER);

						if (zout <= (CUBE_HEIGHT - 1)
								&& (yout <= CUBE_SIZE - 1))
							set_led(layers, x, yout, zout);
					}
				}
			}
		}
		break;
	}
	memset(buffer, 0, sizeof(frame_t) * CUBE_HEIGHT);
	memcpy(buffer, layers, sizeof(frame_t) * CUBE_HEIGHT);
}

void draw_line(frame_t *buffer, point_t start, point_t end) {
	float t = 0;
	int8_t dir_vector[3] = { end.x - start.x, end.y - start.y, end.z - start.z };

	for (t = 0; t <= 1; t += 0.2) {
		uint8_t x =
				round(
						map((start.x+dir_vector[0]*t), 0, CUBE_SIZE-1, 0, CUBE_SIZE - 1));
		uint8_t y =
				round(
						map((start.y+dir_vector[1]*t), 0, CUBE_SIZE-1, 0, CUBE_SIZE - 1));
		uint8_t z =
				round(
						map((start.z+dir_vector[2]*t), 0, CUBE_HEIGHT-1, 0, CUBE_HEIGHT- 1));
		set_led(buffer, x, y, z);
	}
}

void draw_plane(frame_t *buffer, uint8_t plane, uint8_t axis) {
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t z = 0;

	switch (plane) {
	case PLANE_XY:
		for (y = 0; y < CUBE_SIZE; y++)
			for (x = 0; x < CUBE_SIZE; x++)
				set_led(buffer, x, y, axis);
		break;
	case PLANE_XZ:
		for (z = 0; z < CUBE_HEIGHT; z++)
			for (x = 0; x < CUBE_SIZE; x++)
				set_led(buffer, x, axis, z);
		break;
	case PLANE_YZ:
		for (z = 0; z < CUBE_HEIGHT; z++)
			for (y = 0; y < CUBE_SIZE; y++)
				set_led(buffer, axis, y, z);
		break;
	}
}

