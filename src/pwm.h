/*
 * pwm.h
 *
 *  Created on: Feb 6, 2012
 *      Author: cpw9613
 */

#ifndef PWM_H_
#define PWM_H_

#include <stdint.h>
#include <time.h>
#include <pthread.h>

typedef struct{
	uintptr_t handle;
	struct itimerspec timer;
	timer_t timerid;
	pthread_t thread;
	uint32_t highWait;
	uint32_t lowWait;
} pwm_t;

pwm_t *pwm_init(pwm_t*, uint8_t);
void pwm_set(pwm_t*, uint8_t);

#endif /* PWM_H_ */
