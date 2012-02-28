/*
 * accelerometer.h
 *
 *  Created on: Feb 23, 2012
 *      Author: Alex Crawford
 *              Conlan Wesson
 */

#ifndef __ACCELEROMETER_H_
#define __ACCELEROMETER_H_

#include "transform.h"

#define ACCEL_MAX_PERIOD 3334000 // ~300Hz

void accel_init(transfer_args_t *);
float accel_getangle();

#endif /* __ACCELEROMETER_H_ */
