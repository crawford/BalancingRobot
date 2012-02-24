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

#define ANALOG_GAIN		    0x02    // 0V - 5V
#define ANALOG_HIGH_CHANNEL	0x02
#define ANALOG_LOW_CHANNEL	0x00
#define ANALOG_CHANNEL_RANGE ((ANALOG_HIGH_CHANNEL << 4) | ANALOG_LOW_CHANNEL)

static uintptr_t baseHandle;
static uintptr_t lsbHandle;
static uintptr_t msbHandle;
static uintptr_t atdHandle;
static uintptr_t aigHandle;
static uintptr_t intHandle;
static uintptr_t fthHandle;

void accelerometer_isr() {
}

void init_accelerometer() {
	baseHandle = mmap_device_io(PORT_LENGTH, BASE_ADDRESS);
	lsbHandle = mmap_device_io(PORT_LENGTH, LSB_ADDRESS);
	msbHandle = mmap_device_io(PORT_LENGTH, MSB_ADDRESS);
	atdHandle = mmap_device_io(PORT_LENGTH, ATD_ADDRESS);
	aigHandle = mmap_device_io(PORT_LENGTH, AIG_ADDRESS);
	intHandle = mmap_device_io(PORT_LENGTH, INT_ADDRESS);
	fthHandle = mmap_device_io(PORT_LENGTH, FIFO_THR_ADDRESS);

	// Set the FIFO threshold to 3
	out8(fthHandle, 3);

	// Initialize ATD Channel
	out8(atdHandle, ANALOG_CHANNEL_RANGE);

	// Initialize ATD Gain
	out8(aigHandle, ANALOG_GAIN);

	// Allow configurations to settle
	usleep(10);

	// Enable analog interrupts
	out8(intHandle, in8(intHandle) | 0x09);
	out8(intHandle, in8(intHandle) & ~0x10);
	register_isr(&accelerometer_isr, ADC_ISR);
}
