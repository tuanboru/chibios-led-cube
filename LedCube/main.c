#include "ch.h"
#include "hal.h"

#include "types.h"
#include "frame_io.h"
#include "bitfields.h"
#include "func_presets.h"
#include "lis302dl.h"

/*
 * Connections:
 *
 * SPI (column select):
 * 	CLK		-->  PB15 (Red right)
 * 	RCLK	-->  PB12 (Orange center)
 * 	SI		-->  PB13 (Red left)
 *
 * GPIO (layer select):
 *  L0      -->  PE2 (green first down)
 *  L1      -->  PE3 (blu second)
 *  L2      -->  PE4 (white fifth)
 *  L3      -->  PE5 (gray  fourth)
 *  L4      -->  PE6 (violet third)
 *
 * GND      -->  (black)
 * VCC (5V) -->  (white)
 */

/*
 * Flat cable connections:
 *
 * Latch0:   A0 B0 C0 D0 E0 A1 B1 C1
 * Latch1:   D1 E1 A2 B2 C2 D2 E2 A3
 * Latch2:   B3 C3 D3 E3 A4 B4 C4 D4
 * Latch3:   E4
 *
 *  ----------------------------------------------------
 * | A0 A1 B1 C1 D1 E1 A2 B2 C2 D2 E2 A3 B3 C3 D3 E3 A4 |
 * | B0 C0 D0 E0 xx xx xx L0 L1 L2 L3 xx xx E4 D4 C4 B4 |
 *  ----------------------------------------------------
 *
 *    0   1   2   3   4
 * A  x   x   x   x   x
 * B  x   x   x   x   x
 * C  x   x   x   x   x
 * D  x   x   x   x   x
 * E  x   x   x   x   x
 *
 * (0,0,0) ... (0,1,0)
 *   .            .
 *   .            .
 *   .            .
 * (1,0,0) ... (1,1,0)
 *
 *
 */

static bool_t do_accel = FALSE;
static bool_t stop_anim = FALSE;

/*
 * SPI1 configuration structure.
 */
static const SPIConfig spi1cfg =
		{ NULL, /* HW dependent part.*/GPIOE, GPIOE_CS_SPI, SPI_CR1_BR_0
				| SPI_CR1_BR_1 | SPI_CR1_CPOL | SPI_CR1_CPHA };

/*
 * SPI2 configuration structure.
 */
static const SPIConfig spi2cfg = { 0, /* HW dependent part.*/GPIOB, GPIOB_PIN12,
		SPI_CR1_BR_2 | SPI_CR1_BR_1 };

/* Triggered when the button is pressed or released. The LED4 is set to ON.*/
static void extcb1(EXTDriver *extp, expchannel_t channel) {
	(void) extp;
	(void) channel;

	chSysLockFromIsr();
	stop_anim = TRUE;
	do_accel = !do_accel;
	chSysUnlockFromIsr();
}

static const EXTConfig extcfg = { { { EXT_CH_MODE_FALLING_EDGE
		| EXT_CH_MODE_AUTOSTART | EXT_MODE_GPIOA, extcb1 }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL }, {
		EXT_CH_MODE_DISABLED, NULL }, { EXT_CH_MODE_DISABLED, NULL } } };

static frame_t anim_buffer[CUBE_HEIGHT];
static anim_func_t anims[AVAILABLE_PRESETS];

void do_animation(frame_t *buffer, uint16_t delay) {
	uint8_t refresh = 0;
	systime_t start_time;

	start_time = chTimeNow();
	while (TRUE) {
		select_led(buffer[refresh].layer);

		enable_frame(refresh);

		if (refresh++ == CUBE_HEIGHT)
			refresh = 0;

		if (chTimeNow() - start_time > delay)
			break;

		chThdYield();
	}
}

/*
 *
 */
static WORKING_AREA(waAnimationThread, 512);
static msg_t AnimationThread(void *arg) {
	uint16_t i = 0;
	anim_func_t current_anim;
	systime_t start_time;
	uint8_t anim_index = 0;

	(void) arg;
	chRegSetThreadName("animation");

	init_anim_func(anims);

	start_time = chTimeNow();
	while (TRUE) {
		stop_anim = FALSE;

		if (do_accel)
			anim_index = AVAILABLE_PRESETS - 1;

		current_anim = anims[anim_index];
		/*
		 *  Drawing algorithm with functions
		 */
		for (i = 0; i < current_anim.frames && !stop_anim; i++) {
			create_frame_function(anim_buffer, current_anim.func, i);
			do_animation(anim_buffer, current_anim.delay);
		}

		if (chTimeNow() - start_time > SCHEDULING_INTERVAL && !do_accel) {
			start_time = chTimeNow();
			anim_index = (anim_index + 1) % (AVAILABLE_PRESETS - 1);
		}
	}
	return (msg_t) 0L;
}

/*===========================================================================*/
/* Initialization and main thread.                                           */
/*===========================================================================*/

/*
 * Application entry point.
 */
int main(void) {
	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

	/*
	 * Initializes the SPI driver 1 in order to access the MEMS. The signals
	 * are already initialized in the board file.
	 */
	spiStart(&SPID1, &spi1cfg);

	/* LIS302DL initialization.*/
	lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG1, 0x43);
	lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG2, 0x00);
	lis302dlWriteRegister(&SPID1, LIS302DL_CTRL_REG3, 0x00);

	/*
	 * Activates the EXT driver 1.
	 */
	extStart(&EXTD1, &extcfg);

	/*
	 * SPI2 I/O pins setup.
	 */
	spiStart(&SPID2, &spi2cfg); /* Setup transfer parameters.       */
	palSetPadMode(GPIOB, 13, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
	/* New SCK.     */
	palSetPadMode(GPIOB, 14, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
	/* New MISO.    */
	palSetPadMode(GPIOB, 15, PAL_MODE_ALTERNATE(5) | PAL_STM32_OSPEED_HIGHEST);
	/* New MOSI.    */
	palSetPadMode(GPIOB, 12,
			PAL_MODE_OUTPUT_PUSHPULL | PAL_STM32_OSPEED_HIGHEST);
	/* New CS.      */
	palSetPad(GPIOB, 12);

	/*
	 * Creates the example thread.
	 */
	chThdCreateStatic(waAnimationThread, sizeof(waAnimationThread),
			NORMALPRIO + 10, AnimationThread, 0L);

	/*
	 * Normal main() thread activity.
	 */
	while (TRUE) {
		chThdSleepMilliseconds(500);
	}
}
