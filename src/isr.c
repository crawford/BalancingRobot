/*
 * isr.c
 *
 *  Created on: Feb 23, 2012
 *      Author: abc4471
 */

#include <errno.h>
#include <stdint.h>       // for uintptr_t
#include <sys/mman.h>     // for mmap_device_io()
#include <hw/inout.h>     // for in*() and out*() functions
#include <sys/neutrino.h> // for InterruptAttach
#include <stdio.h>
#include "isr.h"

#define PORT_LENGTH      1
#define BASE_ADDRESS     0x280				// Data Acquisition Base Address
#define STATUS_ADDRESS   (BASE_ADDRESS + 7)	// Analog Operation Status Register
#define IO_INTERRUPT     5

void _default_isr();
const struct sigevent* _isr_io(void* area, int id);

void (*_callbacks[4])() = { &_default_isr, &_default_isr, &_default_isr, &_default_isr };
static uintptr_t statusHandle;

void init_isr() {
	statusHandle = mmap_device_io(PORT_LENGTH, STATUS_ADDRESS);

	//Register the IO interrupt
	if (InterruptAttach(IO_INTERRUPT, &_isr_io, NULL, 0, 0) == -1) {
		perror("InterruptAttach()");
	}

	printf("registered\n");
}

void _default_isr() {}

int register_isr(void (*callback)(), int interrupt_id) {
	if (callback != NULL) {
		_callbacks[interrupt_id] = callback;
		return 0;
	}
	return -1;
}

const struct sigevent* _isr_io(void* area, int id) {
	int status = in8(statusHandle) >> 4;

	if (status & (1 << DMA_ISR)) _callbacks[DMA_ISR]();
	if (status & (1 << TMR_ISR)) _callbacks[TMR_ISR]();
	if (status & (1 << DIO_ISR)) _callbacks[DIO_ISR]();
	if (status & (1 << ADC_ISR)) _callbacks[ADC_ISR]();

	// Reset the interrupt
	out8(BASE_ADDRESS, 0x0F);

	return NULL;
}
