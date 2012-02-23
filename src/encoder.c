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

//Encoder -> pins 17 and 18

volatile unsigned int count = 0;
volatile unsigned char prev = 0;

void encoder_isr() {
	printf("en isr\n");
	unsigned char port = in8(PORT_C_ADDRESS) & 0x03;

	if (prev == 0x00 && port == 0x01) {
		atomic_add(&count, 1);
	} else if (prev == 0x00 && port == 0x02) {
		atomic_sub(&count, 1);
	}

	// This isn't necessarily atomic
	prev = port;

	// Reset the IO interrupt
	out8(BASE_ADDRESS, 0x02);
}

int init_encoder() {
	//Request IO privileges
	ThreadCtl(_NTO_TCTL_IO, 0);

	// Set DIOCTR = 1 (enabled C4-C7)
	//Enabled input on C high
	out8(DIR_ADDRESS, in8(DIR_ADDRESS) | 0x84);

	//Enabled IO interrupts
	out8(DMA_CONTROL_ADDRESS, 0x02);

	init_isr(&encoder_isr);

#if TEST_ENCODER
	while (1) {
		printf("%d\n", count);
		sleep(0.5);
	}
#endif
	return 0;
}
