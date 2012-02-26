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
#define FIFO_DEP_ADDRESS (BASE_ADDRESS + 6)	// FIFO Depth Register
#define STATUS_ADDRESS   (BASE_ADDRESS + 7)	// Status Register

#define ANALOG_GAIN		     0x06    // 0V - 5V
#define ANALOG_HIGH_CHANNEL	 0x02
#define ANALOG_LOW_CHANNEL	 0x01
#define ANALOG_CHANNEL_RANGE ((ANALOG_HIGH_CHANNEL << 4) | ANALOG_LOW_CHANNEL)

#define AD_START   0x80
#define AD_STATUS  0x80
#define FIFO_RESET 0x10
#define FIFO_THRESHOLD 20

#define Y_MIN 12300
#define Y_MAX 32300
#define Z_MIN 8300
#define Z_MAX 29300

#define ACCEL_PERIOD_NS 10000000
#define ATD_TIMEOUT     10000

#define NUM_SAMPLES 10

//#define DEBUG

static uintptr_t baseHandle;
static uintptr_t lsbHandle;
static uintptr_t msbHandle;
static uintptr_t atdHandle;
static uintptr_t aigHandle;
static uintptr_t intHandle;
static uintptr_t fthHandle;
static uintptr_t statusHandle;
static uintptr_t fifoDepthHandle;

static float current_angle = 0;
static float samples[NUM_SAMPLES];
static int pointer = 0;

/**
 * Thread to read values from the accelerometer.
 */
void accel_thread(union sigval s) {
	int waitReading;
	int converted = 0;

	uint16_t value = 0;

	for (waitReading = 0; waitReading < ATD_TIMEOUT; waitReading++) {
		if ((in8(aigHandle) & AD_STATUS) == 0) {
			converted = 1;
			break;
		}
	}

	if (converted) {
		value = in8(lsbHandle);
		value |= in8(msbHandle) << 8;

		float Y = (value - (Y_MIN + Y_MAX)/2) * M_PI / (Y_MAX - Y_MIN);


		value = in8(lsbHandle);
		value |= in8(msbHandle) << 8;

		float Z = (value - (Z_MIN + Z_MAX)/2) * M_PI / (Z_MAX - Z_MIN);

		samples[pointer] = atan2f(Z, Y);
		pointer++;
		if(pointer >= NUM_SAMPLES){
			pointer = 0;
		}
		float sum = 0;
		int i;
		for(i = 0; i < NUM_SAMPLES; i++){
			sum += samples[i];
		}
		current_angle = sum/((float)NUM_SAMPLES);

#ifdef DEBUG
		printf("Y: %0.2f  Z: %0.2f    Angle: %0.2f\n", Y, Z, current_angle);
#endif
	}

	//Start the next conversion
	out8(baseHandle, AD_START);
}

/**
 * Initialize the accelerometer.
 */
void accel_init() {
	baseHandle = mmap_device_io(PORT_LENGTH, BASE_ADDRESS);
	lsbHandle = mmap_device_io(PORT_LENGTH, LSB_ADDRESS);
	msbHandle = mmap_device_io(PORT_LENGTH, MSB_ADDRESS);
	atdHandle = mmap_device_io(PORT_LENGTH, ATD_ADDRESS);
	aigHandle = mmap_device_io(PORT_LENGTH, AIG_ADDRESS);
	intHandle = mmap_device_io(PORT_LENGTH, INT_ADDRESS);
	fthHandle = mmap_device_io(PORT_LENGTH, FIFO_THR_ADDRESS);
	statusHandle = mmap_device_io(PORT_LENGTH, STATUS_ADDRESS);
	fifoDepthHandle = mmap_device_io(PORT_LENGTH, FIFO_DEP_ADDRESS);

	// Set the FIFO threshold
	out8(fthHandle, FIFO_THRESHOLD);
	out8(baseHandle, FIFO_RESET);

	// Initialize ATD Channel
	out8(atdHandle, ANALOG_CHANNEL_RANGE);

	// Initialize ATD Gain
	out8(aigHandle, ANALOG_GAIN);

	// Allow configurations to settle
	usleep(10);

	out8(intHandle, in8(intHandle) & ~0x01);

	// Enable analog interrupts
	/*out8(intHandle, in8(intHandle) | 0x09);
	out8(intHandle, in8(intHandle) & ~0x10);
	register_isr(&accelerometer_isr, ADC_ISR);*/

	// Initiate the first conversion
	out8(baseHandle, AD_START);

	// Setup the ATD timer.
	struct sigevent event;
	SIGEV_THREAD_INIT(&event, &accel_thread, NULL, NULL);

	timer_t timer;
	timer_create(CLOCK_REALTIME, &event, &timer);

	struct timespec millisec = { .tv_sec = 0, .tv_nsec = ACCEL_PERIOD_NS };
	struct itimerspec time_run = { .it_value = millisec, .it_interval = millisec };
	timer_settime(timer, 0, &time_run, NULL);
}

/**
 * Returns the current angle of the accelerometer.
 * @return The angle in radians such that PI/2 is horizontal.
 */
float accel_getangle(){
	return current_angle;
}
