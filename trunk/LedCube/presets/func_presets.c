/*
 * func_presets.c
 *
 *  Created on: 16/mag/2013
 *      Author: Luca
 */
#include "func_presets.h"
#include "frame_io.h"
#include "types.h"
#include "bitfields.h"
#include "string.h"
#include "utils.h"
#include "ch.h"
#include "hal.h"
#include "lis302dl.h"

static char *string = "5x5cubeled";

void f_chars(frame_t *buffer, uint16_t frame) {

	memset(buffer, 0, sizeof(frame_t) * CUBE_HEIGHT);
	if (frame % 13 < 2) {
		draw_char(buffer, string[frame / 13], frame % 13, PLANE_XZ);
	} else if (frame % 13 >= 2 && frame % 13 <= 10) {
		draw_char(buffer, string[frame / 13], 2, PLANE_XZ);
		rotate_axis(buffer, ((frame % 13) - 2) * 45, AXIS_Z);
	} else
		draw_char(buffer, string[frame / 13], frame % 13 - 8, PLANE_XZ);
}

void f_lines(frame_t *buffer, uint16_t frame) {
	point_t start;
	point_t end;

	memset(buffer, 0, sizeof(frame_t) * CUBE_HEIGHT);

	if (frame % 2 == 0) {
		start.x = rand() % CUBE_SIZE;
		start.y = rand() % CUBE_SIZE;
		start.z = 0;
		end.x = start.x;
		end.y = start.y;
		end.z = CUBE_HEIGHT - 1;
	} else {
		start.x = rand() % CUBE_SIZE;
		start.y = 0;
		start.z = rand() % CUBE_SIZE;
		end.x = start.x;
		end.y = CUBE_SIZE - 1;
		end.z = start.z;
	}
	draw_line(buffer, start, end);
}

void f_plane_rotate(frame_t *buffer, uint16_t frame) {

	memset(buffer, 0, sizeof(frame_t) * CUBE_HEIGHT);
	if (frame < 8) {
		draw_plane(buffer, PLANE_XZ, ROT_CENTER);
		rotate_axis(buffer, frame * 45, AXIS_Z);
	} else if (frame >= 8 && frame < 16) {
		draw_plane(buffer, PLANE_XY, ROT_CENTER);
		rotate_axis(buffer, frame * 45, AXIS_Y);
	} else {
		draw_plane(buffer, PLANE_XY, ROT_CENTER);
		rotate_axis(buffer, frame * 45, AXIS_X);
	}
}

void f_rand(frame_t *buffer, uint16_t frame) {
	uint8_t x = 0;
	uint8_t z = 0;

	(void) frame;
	for (z = 0; z < CUBE_HEIGHT; z++) {
		buffer[z].layer = 0;
		for (x = 0; x < CUBE_SIZE; x++) {
			uint8_t pattern = rand() % (CUBE_SIZE * CUBE_SIZE);
			BF_SET(buffer[z].layer, pattern, x*CUBE_SIZE, CUBE_SIZE);
		}
	}
}

void f_cubes(frame_t *buffer, uint16_t frame) {
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t z = 0;
	uint8_t a = 0;
	uint8_t b = 0;

	memset(buffer, 0, sizeof(frame_t) * CUBE_HEIGHT);

	frame = frame % 6;
	if (frame < 3) {
		a = CUBE_SIZE - frame;
		b = frame;
	} else if (frame >= 3 && frame < 6) {
		a = frame;
		b = CUBE_SIZE - frame;
	}

	for (x = b; x < a; x++) {
		for (y = b; y < a; y++) {
			for (z = b; z < a; z++) {
				if (z == b || z == a - 1) {
					if ((x == b || x == a - 1) || (y == b || y == a - 1))
						set_led(buffer, x, y, z);
				} else if (z != b && z != a - 1) {
					if ((x == b || x == a - 1) && (y == b || y == a - 1))
						set_led(buffer, x, y, z);
				}
			}
		}
	}
}

void f_rain(frame_t *buffer, uint16_t frame) {

	uint8_t i = 0;
	uint8_t z = 0;
	uint8_t drops = rand() % 4;
	frame_t layers[CUBE_HEIGHT];

	(void) frame;
	memset(layers, 0, sizeof(frame_t) * CUBE_HEIGHT);

	for (z = 0; z < CUBE_HEIGHT - 1; z++)
		layers[z + 1].layer = get_pattern(buffer[z],0);

	for (i = 0; i < drops; i++) {
		uint8_t x = rand() % CUBE_SIZE;
		uint8_t y = rand() % CUBE_SIZE;
		set_led(layers, x, y, 0);
	}

	memset(buffer, 0, sizeof(frame_t) * CUBE_HEIGHT);
	memcpy(buffer, layers, sizeof(frame_t) * CUBE_HEIGHT);
}

void f_plane_boing(frame_t *buffer, uint16_t frame) {
	uint8_t axis = 0;

	memset(buffer, 0, sizeof(frame_t) * CUBE_HEIGHT);

	if (frame < 10) {
		if (frame < CUBE_SIZE)
			axis = frame;
		else
			axis = 10 - frame - 1;

		draw_plane(buffer, PLANE_XZ, axis);
	} else if (frame >= 10 && frame < 20) {
		if (frame - 10 < CUBE_SIZE)
			axis = frame - 10;
		else
			axis = 20 - frame - 1;
		draw_plane(buffer, PLANE_XY, axis);
	} else {
		if (frame - 20 < CUBE_SIZE)
			axis = frame - 20;
		else
			axis = 30 - frame - 1;

		draw_plane(buffer, PLANE_YZ, axis);
	}
}

void f_cosine(frame_t *buffer, uint16_t frame) {
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t z = 0;

	memset(buffer, 0, sizeof(frame_t) * CUBE_HEIGHT);

	for (y = 0; y < CUBE_SIZE; y++) {
		for (x = 0; x < CUBE_SIZE; x++) {
			double zcos = cos(22.5 * (x + frame) * M_PI / 180.0);
			z = round(map(zcos, -1, 1, 0, CUBE_HEIGHT - 1));
			set_led(buffer, x, y, z);
		}
	}

}

void f_ripple(frame_t *buffer, uint16_t frame) {
	uint8_t x = 0;
	uint8_t y = 0;
	uint8_t z = 0;

	memset(buffer, 0, sizeof(frame_t) * CUBE_HEIGHT);

	for (y = 0; y < CUBE_SIZE; y++) {
		for (x = 0; x < CUBE_SIZE; x++) {
			double dist = sqrt(
					pow(map(x, 0, CUBE_SIZE - 1, -M_PI, M_PI), 2)
							+ pow(map(y, 0, CUBE_SIZE - 1,
									-M_PI, M_PI), 2));
			double zsin = sin(frame * 10 * M_PI / 180.0f + dist);
			z = round(map(zsin, -1, 1, 0, CUBE_HEIGHT - 1));
			set_led(buffer, x, y, z);
		}
	}
}

void f_accelerometer(frame_t *buffer, uint16_t frame) {
	int32_t x, y;
	static int8_t xbuf[4], ybuf[4]; /* Last accelerometer data.*/
	unsigned i;

	(void) frame;
	/* Keeping an history of the latest four accelerometer readings.*/
	for (i = 3; i > 0; i--) {
		xbuf[i] = xbuf[i - 1];
		ybuf[i] = ybuf[i - 1];
	}

	/* Reading MEMS accelerometer X and Y registers.*/
	xbuf[0] = (int8_t) lis302dlReadRegister(&SPID1, LIS302DL_OUTX);
	ybuf[0] = (int8_t) lis302dlReadRegister(&SPID1, LIS302DL_OUTY);

	/* Calculating average of the latest four accelerometer readings.*/
	x = ((int32_t) xbuf[0] + (int32_t) xbuf[1] + (int32_t) xbuf[2]
			+ (int32_t) xbuf[3]) / 4;
	y = ((int32_t) ybuf[0] + (int32_t) ybuf[1] + (int32_t) ybuf[2]
			+ (int32_t) ybuf[3]) / 4;

	memset(buffer, 0, sizeof(frame_t) * CUBE_HEIGHT);
	draw_plane(buffer, PLANE_XY, ROT_CENTER);

	rotate_axis(buffer, -((int16_t)map(y, -128, 128, 180, -180))-90, AXIS_X);
	rotate_axis(buffer, ((int16_t)map(x, -128, 128, 180, -180))-90, AXIS_Y);
}

void init_anim_func(anim_func_t *anims) {
	uint8_t i = 0;

	for (i = 0; i < AVAILABLE_PRESETS; i++) {
		switch (i) {
		case 0:
			anims[i].delay = 150;
			anims[i].frames = strlen(string) * 13;
			anims[i].func = f_chars;
			break;
		case 1:
			anims[i].delay = 50;
			anims[i].frames = 50;
			anims[i].func = f_lines;
			break;
		case 2:
			anims[i].delay = 30;
			anims[i].frames = 36;
			anims[i].func = f_ripple;
			break;
		case 3:
			anims[i].delay = 50;
			anims[i].frames = 32;
			anims[i].func = f_cosine;
			break;
		case 4:
			anims[i].delay = 50;
			anims[i].frames = 30;
			anims[i].func = f_plane_boing;
			break;
		case 5:
			anims[i].delay = 100;
			anims[i].frames = 40;
			anims[i].func = f_rain;
			break;
		case 6:
			anims[i].delay = 100;
			anims[i].frames = 60;
			anims[i].func = f_cubes;
			break;
		case 7:
			anims[i].delay = 100;
			anims[i].frames = 10;
			anims[i].func = f_rand;
			break;
		case 8:
			anims[i].delay = 100;
			anims[i].frames = 24;
			anims[i].func = f_plane_rotate;
			break;
		case 9:
			anims[i].delay = 50;
			anims[i].frames = 1;
			anims[i].func = f_accelerometer;
			break;
		}
	}
}
