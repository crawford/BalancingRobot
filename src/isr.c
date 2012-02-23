/*
 * isr.c
 *
 *  Created on: Feb 23, 2012
 *      Author: abc4471
 */

#include <errno.h>
#include <sys/neutrino.h> /* for ThreadCtl() */
#include <stdio.h>
#include "isr.h"

void (*_callbacks[MAX_CALLBACKS])();
unsigned int _num_callbacks = 0;

void init_isr(void (*callback)()) {
	//Register the IO interrupt
	if (InterruptAttach(IO_INTERRUPT, &_isr_io, NULL, 0, 0) == -1) {
		perror("InterruptAttach()");
	}

	if (callback != NULL) {
		if (_num_callbacks == MAX_CALLBACKS) {
			errno = EINVAL;
			perror("Too many callbacks");
			return;
		}
		_callbacks[_num_callbacks] = callback;
		_num_callbacks++;
	}
}

const struct sigevent* _isr_io(void* area, int id) {
	int i;
	for (i = 0; i < _num_callbacks; i++) {
		_callbacks[i]();
	}

	return NULL;
}
