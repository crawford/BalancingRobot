/*
 * transform.h
 *
 *  Created on: Jan 25, 2012
 *      Author: Alex Crawford
 *              Conlan Wesson
 */

#ifndef __TRANSFORM_H_
#define __TRANSFORM_H_

#include <pthread.h>
#include <stdio.h>
#include "motor.h"

typedef struct {
	double inputs[3];
	double output;
	double ref;
	long out_interval;
	long in_interval;
	pthread_mutex_t *mutex;
	FILE *fd;
	motor_t *motor;
} transfer_args_t;

#endif /* __TRANSFORM_H_ */
