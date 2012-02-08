/*
 * pwm.c
 *
 *  Created on: Feb 6, 2012
 *      Author: cpw9613
 */

#include "pwm.h"

#include <pthread.h>
#include <errno.h>
#include <unistd.h>       /* for sleep() */
#include <stdint.h>       /* for uintptr_t */
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/neutrino.h> /* for ThreadCtl() */
#include <sys/mman.h>     /* for mmap_device_io() */
#include <stdio.h>

#define PWM_PERIOD 50000

#define PORT_LENGTH 1
#define CODE_TIMER 1

#define BASE_ADDRESS 0x280
#define DIR_ADDRESS (BASE_ADDRESS+11)
#define PORT_A_ADDRESS (BASE_ADDRESS+8)

typedef struct{
	// Message to and from client.
	int messageType;
	// Optional data depending on the message.
	int messageData;
} ClientMessageT;

typedef union{
	// A message can be either from a client or a pulse.
	ClientMessageT msg;
	struct _pulse pulse;
} MessageT;

static int chid;

void getPulse(){
	MessageT msg;
	for(;;){
		int rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);

		// See if the message is a pulse.
		if(rcvid == 0){
			return;
		}else{
			perror("getPulse()");
		}
	}
}

void *pwm_thread(void *arg){
	pwm_t *pwm = (pwm_t*)arg;
	for(;;){
		//SET PORT HIGH
		out8(pwm->handle, in8(pwm->handle) | 0x01);

		// Setup the timer data.
		pwm->timer.it_value.tv_nsec = pwm->highWait;
		// Start the timer.
		timer_settime(pwm->timerid, 0, &pwm->timer, NULL);
		getPulse();

		//SET PORT LOW
		out8(pwm->handle, in8(pwm->handle) & ~0x01);

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
	int coid;                     // connection ID.

	pwm->highWait = duty * PWM_PERIOD / 255;
	pwm->lowWait = PWM_PERIOD - pwm->highWait;

	uintptr_t dirHandle = mmap_device_io(PORT_LENGTH, DIR_ADDRESS);
	pwm->handle = mmap_device_io(PORT_LENGTH, PORT_A_ADDRESS);

	out8(dirHandle, in8(dirHandle) & ~0x10);

	if((chid = ChannelCreate(0)) == -1){
		fprintf(stderr, "Couldn't create channel!\n");
		perror(NULL);
		return NULL;
	}

	// Create connection back to this process.
	coid = ConnectAttach(0, 0, chid, 0, 0);
	if(coid == -1){
		fprintf(stderr, "Couldn't ConnectAttach!\n");
		perror(NULL);
		return NULL;
	}

	// Setup the pulse event.
	SIGEV_PULSE_INIT(&event, coid, SIGEV_PULSE_PRIO_INHERIT, CODE_TIMER, 0);

	// Create the timer event binding.
	if(timer_create(CLOCK_REALTIME, &event, &pwm->timerid) == -1){
		fprintf(stderr, "Can't timer_create, errno, %d\n", errno);
		perror(NULL);
		return NULL;
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
	pwm->lowWait = PWM_PERIOD - pwm->highWait;
}
