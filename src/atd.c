/*
 * atd.c
 *
 *  Created on: Jan 20, 2012
 *      Author: Alex Crawford
 *              Conlan Wesson
 */

#include "atd.h"

#include <unistd.h>       // for sleep()
#include <stdint.h>       // for uintptr_t
#include <hw/inout.h>     // for in*() and out*() functions
#include <sys/neutrino.h> // for ThreadCtl()
#include <time.h>
#include <sys/mman.h>     // for mmap_device_io()

#define PORT_LENGTH 1

#define BASE_ADDRESS	0x280				// Data Acquisition Base Address
#define LSB_ADDRESS		BASE_ADDRESS		// Least Significant Bytes
#define MSB_ADDRESS		(BASE_ADDRESS + 1)	// Most Significant Bytes
#define ATD_ADDRESS		(BASE_ADDRESS + 2)	// ATD Channel Register
#define AIG_ADDRESS		(BASE_ADDRESS + 3)	// Analog Input Gain Register
#define INT_ADDRESS		(BASE_ADDRESS + 4)	// Interrupt Control Register

#define ANALOG_CHANNEL	0    // Channel 0
#define ANALOG_GAIN		0    // +/- 10V

static uintptr_t baseHandle;
static uintptr_t lsbHandle;
static uintptr_t msbHandle;
static uintptr_t atdHandle;
static uintptr_t aigHandle;
static uintptr_t intHandle;

/**
 * Convert input value to -1 - 1.
 */
static inline double ADC_to_value(int16_t value) {
	return value / 32768.0;
}

/**
 * Thread to read values from the ATD.
 */
void atd_thread(union sigval s){
	transfer_args_t *args = s.sival_ptr;

	pthread_mutex_lock(args->mutex);
	args->inputs[2] = args->inputs[1];
	args->inputs[1] = args->inputs[0];
	args->inputs[0] = args->ref - ADC_to_value(atd_read());
	pthread_mutex_unlock(args->mutex);
}

/**
 * Initialize the ATD.
 */
void atd_init(transfer_args_t *args){
	baseHandle = mmap_device_io(PORT_LENGTH, BASE_ADDRESS);
	lsbHandle = mmap_device_io(PORT_LENGTH, LSB_ADDRESS);
	msbHandle = mmap_device_io(PORT_LENGTH, MSB_ADDRESS);
	atdHandle = mmap_device_io(PORT_LENGTH, ATD_ADDRESS);
	aigHandle = mmap_device_io(PORT_LENGTH, AIG_ADDRESS);
	intHandle = mmap_device_io(PORT_LENGTH, INT_ADDRESS);

	// Initialize ATD Channel
	out8(atdHandle, ANALOG_CHANNEL);

	// Initialize ATD Gain
	out8(aigHandle, ANALOG_GAIN);

	// Allow configurations to settle
	usleep(100);

	// Disabling analog interrupts
	/*uint8_t interruptStatus = in8(intHandle);
	interruptStatus = interruptStatus & 0xFE;
	out8(intHandle, interruptStatus);*/

	out8(intHandle, in8(intHandle) | 0x01);

	// Setup the ATD timer.
	struct sigevent event;
	SIGEV_THREAD_INIT(&event, &atd_thread, args, NULL);

	timer_t timer;
	timer_create(CLOCK_REALTIME, &event, &timer);

	struct timespec millisec = { .tv_sec = 0, .tv_nsec = args->in_interval };
	struct itimerspec time_run = { .it_value = millisec, .it_interval = millisec };
	timer_settime(timer, 0, &time_run, NULL);
}

/**
 * Read a value from the ATD.
 * @return -32768 to 32767
 */
int16_t atd_read(){
	int waitReading;
	int converted = 0;

	uint8_t lsbData = 0;
	uint8_t msbData = 0;

	out8(baseHandle, 0x80);

	for (waitReading = 0; waitReading < 10000; waitReading++) {
		if ((in8(aigHandle) & 0x80) == 0) {
			converted = 1;
			break;
		}
	}

	if (converted) {
		lsbData = in8(lsbHandle);
		msbData = in8(msbHandle);
	}

	return (msbData << 8) | lsbData;
}
