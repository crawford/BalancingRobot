/*
 * transform.h
 *
 *  Created on: Jan 25, 2012
 *      Author: Alex Crawford
 *              Conlan Wesson
 */

#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include <pthread.h>
#include <stdio.h>

typedef struct {
	double inputs[3];
	double output;
	double ref;
	long out_interval;
	long in_interval;
	long calc_interval;
	pthread_mutex_t *mutex;
	FILE *fd;
} transfer_args_t;

#endif /* TRANSFORM_H_ */
