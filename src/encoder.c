/*
 * encoder.c
 *
 *  Created on: Feb 14, 2012
 *      Author: abc4471
 */

#include <unistd.h>       /* for sleep() */
#include <stdint.h>       /* for uintptr_t */
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/mman.h>     /* for mmap_device_io() */
#include <sys/neutrino.h> /* for ThreadCtl() */
#include <stdio.h>
#include <atomic.h>
#include "encoder.h"
#include "isr.h"

//#define TEST_ENCODER

//Encoder -> pins 17 and 18

volatile unsigned int count = 0;
volatile unsigned char prev = 0;

void encoder_isr() {
	unsigned char port = in8(PORT_C_ADDRESS) & 0x03;

	if (prev == 0x00 && port == 0x01) {
		atomic_add(&count, 1);
	} else if (prev == 0x00 && port == 0x02) {
		atomic_sub(&count, 1);
	}

	// This isn't necessarily atomic
	prev = port;

	atomic_add(&count, 1);
}

int init_encoder() {
	// Set DIOCTR = 1 (enabled C4-C7)
	//Enabled input on C high
	out8(DIR_ADDRESS, in8(DIR_ADDRESS) | 0x84);

	//Enabled IO interrupts
	out8(DMA_CONTROL_ADDRESS, 0x02);

	register_isr(&encoder_isr, DIO_ISR);

#ifdef TEST_ENCODER
	while (1) {
		printf("%d\n", count);
		sleep(1);
	}
#endif
	return 0;
}
