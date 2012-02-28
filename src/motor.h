/*
 * motor.h
 *
 *  Created on: Feb 19, 2012
 *      Author: Alex Crawford
 *              Conlan Wesson
 */

#ifndef __MOTOR_H_
#define __MOTOR_H_

#include <stdint.h>
#include "pwm.h"

typedef struct {
	pwm_t* pwm;
} motor_t;

motor_t* motor_init(motor_t*, pwm_t*);
void motor_forward(motor_t*, uint8_t);
void motor_reverse(motor_t*, uint8_t);
void motor_brake(motor_t*);
void motor_free(motor_t*);

#endif /* __MOTOR_H_ */
