/*
 * dio.c
 *
 *  Created on: Feb 27, 2012
 *      Author: cpw9613
 */

#include "dio.h"

#include <atomic.h>
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/mman.h>     /* for mmap_device_io() */

#define PORT_LENGTH 1

#define PORT_A_DIR_BIT 0x10

#define BASE_ADDRESS   0x280
#define DIR_ADDRESS    (BASE_ADDRESS+11)
#define PORT_A_ADDRESS (BASE_ADDRESS+8)

static uintptr_t dirHandle;
static uintptr_t portHandle;
static uint8_t value = 0;

void dio_init(){
	dirHandle = mmap_device_io(PORT_LENGTH, DIR_ADDRESS);
	portHandle = mmap_device_io(PORT_LENGTH, PORT_A_ADDRESS);

	// Set port A as output.
	out8(dirHandle, in8(dirHandle) & ~PORT_A_DIR_BIT);

	out8(portHandle, value);
}

void dio_set(uint8_t mask){
	atomic_set((unsigned*)&value, mask);
	out8(portHandle, value);
}

void dio_clear(uint8_t mask){
	atomic_clr((unsigned*)&value, mask);
	out8(portHandle, value);
}
