/*
 * pwm.c
 *
 *  Created on: Feb 6, 2012
 *      Author: cpw9613
 */

#include "pwm.h"

#include <pthread.h>

#define PWM_PERIOD 20000000

void *pwm_thread(void *arg){
	pwm_t *pwm = (pwm_t*)arg;
	for(;;){
		//SET PORT HIGH
		out8(pwm->handle, 0xFF);

		// Setup the timer data.
		pwm->timer.it_value.tv_nsec = pwm->highWait;
		// Start the timer.
		timer_settime(pwm->timerid, 0, &pwm->timer, NULL);
		getPulse();

		//SET PORT LOW
		out8(pwm->handle, 0x00);

		// Setup the timer data.
		pwm->timer.it_value.tv_nsec = pwm->lowWait;
		// Start the timer.
		timer_settime(pwm->timerid, 0, &pwm->timer, NULL);
		getPulse();
	}

	return NULL;
}

pwm_t *pwm_init(pwm_t *pwm, uint8_t duty){
	struct sigevent event;        // event to deliver.
	struct _clockperiod clock;    // clock period struct.
	int coid;                     // connection ID.

	pwm->highWait = duty * PWM_PERIOD / 255;
	pwm->lowWait = PWM_PERIOD - highWait;

	if((chid = ChannelCreate(0)) == -1){
		fprintf(stderr, "Couldn't create channel!\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	// Create connection back to this process.
	coid = ConnectAttach(0, 0, chid, 0, 0);
	if(coid == -1){
		fprintf(stderr, "Couldn't ConnectAttach!\n");
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	// Adjust the clock period.
	clock.nsec = NANO_SLEEP / 2;    // desired clock period in nanoseconds.
	clock.fract = 0;                // MUST be zero, according to ClockPeriod() documentation.
	if(ClockPeriod(CLOCK_REALTIME, &clock, NULL, 0) == -1){
		fprintf(stderr, "Can't set clock period, errno, %d\n", errno);
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	// Setup the pulse event.
	SIGEV_PULSE_INIT(&event, coid, SIGEV_PULSE_PRIO_INHERIT, CODE_TIMER, 0);

	// Create the timer event binding.
	if(timer_create(CLOCK_REALTIME, &event, &pwm->timerid) == -1){
		fprintf(stderr, "Can't timer_create, errno, %d\n", errno);
		perror(NULL);
		exit(EXIT_FAILURE);
	}

	// Setup the timer data.
	pwm->timer.it_value.tv_sec = 0;
	pwm->timer.it_value.tv_nsec = 0;
	pwm->timer.it_interval.tv_sec = 0;
	pwm->timer.it_interval.tv_nsec = 0;

	pthread_create(&(pwm->thread), NULL, pwm_thread, pwm);

	return pwm;
}

void pwm_set(pwm_t *pwm, uint8_t duty){
	pwm->highWait = duty * PWM_PERIOD / 255;
	pwm->lowWait = PWM_PERIOD - highWait;
}
