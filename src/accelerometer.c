/*
 * accelerometer.c
 *
 *  Created on: Feb 23, 2012
 *      Author: abc4471
 */

#include <stdio.h>
#include <stdint.h>       // for uintptr_t
#include <hw/inout.h>     // for in*() and out*() functions
#include <unistd.h>       // for sleep()
#include <sys/mman.h>     // for mmap_device_io()
#include <time.h>
#include <math.h>
#include "accelerometer.h"
#include "isr.h"

#define PORT_LENGTH     1

#define BASE_ADDRESS     0x280				// Data Acquisition Base Address
#define LSB_ADDRESS      BASE_ADDRESS		// Least Significant Bytes
#define MSB_ADDRESS      (BASE_ADDRESS + 1)	// Most Significant Bytes
#define ATD_ADDRESS      (BASE_ADDRESS + 2)	// ATD Channel Register
#define AIG_ADDRESS      (BASE_ADDRESS + 3)	// Analog Input Gain Register
#define INT_ADDRESS      (BASE_ADDRESS + 4)	// Interrupt Control Register
#define FIFO_THR_ADDRESS (BASE_ADDRESS + 5)	// FIFO Threshold Control Register
#define STATUS_ADDRESS   (BASE_ADDRESS + 7)	// Status Register

#define ANALOG_GAIN		    0x02    // 0V - 5V
#define ANALOG_HIGH_CHANNEL	0x02
#define ANALOG_LOW_CHANNEL	0x01
#define ANALOG_CHANNEL_RANGE ((ANALOG_HIGH_CHANNEL << 4) | ANALOG_LOW_CHANNEL)

#define X_MIN 9400
#define X_MAX 31400
#define Y_MIN 12300
#define Y_MAX 32300
#define Z_MIN 8300
#define Z_MAX 29300

#define PI 3.141592653589
#define DEBUG

static uintptr_t baseHandle;
static uintptr_t lsbHandle;
static uintptr_t msbHandle;
static uintptr_t atdHandle;
static uintptr_t aigHandle;
static uintptr_t intHandle;
static uintptr_t fthHandle;
static uintptr_t statusHandle;


int16_t axis_read();

/**
 * Thread to read values from the accelerometer.
 */
void accel_thread(union sigval s) {
	int channel = in8(statusHandle) & 0x0F;
	float Y;
	float Z;
	if (channel == 1) {
		Y = (axis_read() - (Y_MIN + Y_MAX)/2) * PI / (Y_MAX - Y_MIN);
		Z = (axis_read() - (Z_MIN + Z_MAX)/2) * PI / (Z_MAX - Z_MIN);
	} else {
		Z = (axis_read() - (Z_MIN + Z_MAX)/2) * PI / (Z_MAX - Z_MIN);
		Y = (axis_read() - (Y_MIN + Y_MAX)/2) * PI / (Y_MAX - Y_MIN);
	}
#ifdef DEBUG
	printf("Y: %0.2f  Z: %0.2f    Angle: %0.2f\n", Y/PI, Z/PI, atan2f(Z, Y)/PI*180);
#endif
}

void init_accelerometer() {
	baseHandle = mmap_device_io(PORT_LENGTH, BASE_ADDRESS);
	lsbHandle = mmap_device_io(PORT_LENGTH, LSB_ADDRESS);
	msbHandle = mmap_device_io(PORT_LENGTH, MSB_ADDRESS);
	atdHandle = mmap_device_io(PORT_LENGTH, ATD_ADDRESS);
	aigHandle = mmap_device_io(PORT_LENGTH, AIG_ADDRESS);
	intHandle = mmap_device_io(PORT_LENGTH, INT_ADDRESS);
	fthHandle = mmap_device_io(PORT_LENGTH, FIFO_THR_ADDRESS);
	statusHandle = mmap_device_io(PORT_LENGTH, STATUS_ADDRESS);

	// Set the FIFO threshold to 3
	out8(fthHandle, 3);

	// Initialize ATD Channel
	out8(atdHandle, ANALOG_CHANNEL_RANGE);

	// Initialize ATD Gain
	out8(aigHandle, ANALOG_GAIN);

	// Allow configurations to settle
	usleep(10);

	// Enable analog interrupts
	/*out8(intHandle, in8(intHandle) | 0x09);
	out8(intHandle, in8(intHandle) & ~0x10);
	register_isr(&accelerometer_isr, ADC_ISR);*/

	// Setup the ATD timer.
	struct sigevent event;
	SIGEV_THREAD_INIT(&event, &accel_thread, NULL, NULL);

	timer_t timer;
	timer_create(CLOCK_REALTIME, &event, &timer);

	struct timespec millisec = { .tv_sec = 0, .tv_nsec = 100000000 };
	struct itimerspec time_run = { .it_value = millisec, .it_interval = millisec };
	timer_settime(timer, 0, &time_run, NULL);
}

/**
 * Read a value from the ATD.
 * @return -32768 to 32767
 */
int16_t axis_read() {
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
