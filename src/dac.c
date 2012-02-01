/*
 * dac.c
 *
 *  Created on: Jan 20, 2012
 *      Author: Alex Crawford
 *              Conlan Wesson
 */

#include "DAC.h"

#include <pthread.h>
#include <stdint.h>       // for uintptr_t
#include <hw/inout.h>     // for in*() and out*() functions
#include <sys/mman.h>     // for mmap_device_io()
#include <sys/neutrino.h> // for ThreadCtl()

#define PORT_LENGTH  1

#define BASE_ADDRESS 0x280
#define LSB_ADDRESS  (BASE_ADDRESS + 6)
#define MSB_ADDRESS  (BASE_ADDRESS + 7)

#define CHANNEL      0

//static uintptr_t baseHandle;
static uintptr_t lsbHandle;
static uintptr_t msbHandle;

/**
 * Convert output value to 0-4096.
 */
static inline uint16_t value_to_DAC(double value) {
	if (value > 1) value = 1;
	if (value < -1) value = -1;
	int t = (value * 2048.0) + 2048;
	if (t > 4095) t = 4095;
	if (t < 0) t = 0;
	return t;
}

/**
 * Thread to output voltages via the DAC.
 */
void dac_thread(union sigval s){
	transfer_args_t *args = s.sival_ptr;

	dac_write(value_to_DAC(args->output));
}

/**
 * Initialize the DAC.
 */
void dac_init(transfer_args_t *args){
	lsbHandle = mmap_device_io(PORT_LENGTH, LSB_ADDRESS);
	msbHandle = mmap_device_io(PORT_LENGTH, MSB_ADDRESS);

	struct sigevent event;
	SIGEV_THREAD_INIT(&event, &dac_thread, args, NULL);

	timer_t timer;
	timer_create(CLOCK_REALTIME, &event, &timer);

	struct timespec millisec = { .tv_sec = 0, .tv_nsec = args->out_interval };
	struct itimerspec time_run = { .it_value = millisec, .it_interval = millisec };
	timer_settime(timer, 0, &time_run, NULL);
}

/**
 * Write a value to the DAC.
 * @param value 0-4096
 */
void dac_write(uint16_t value){
	uint8_t msb = ((value & 0x0F00) >> 8) | ((CHANNEL & 0x03) << 6);
	uint8_t lsb = value & 0x00FF;

	out8(lsbHandle, lsb);
	out8(msbHandle, msb);
}
