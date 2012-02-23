/*
 * accelerometer.c
 *
 *  Created on: Feb 23, 2012
 *      Author: abc4471
 */

#include <stdio.h>
#include "accelerometer.h"
#include "isr.h"

void accelerometer_isr() {
	printf("ac isr\n");
}

void init_accelerometer() {

	init_isr(&accelerometer_isr);
}
