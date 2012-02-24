/*
 * PID.c
 *
 *  Created on: Jan 20, 2012
 *      Author: Alex Crawford
 *              Conlan Wesson
 */
#include <hw/inout.h>     // for in*() and out*() functions

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/neutrino.h> // for ThreadCtl()
#include <sys/siginfo.h>
#include "atd.h"
#include "dac.h"
#include "transform.h"
#include "pwm.h"
#include "encoder.h"
#include "accelerometer.h"
#include "isr.h"

#define INPUT_BUF_LEN 50

#define OUTPUT_INTERVAL_NS 10000000
#define INPUT_INTERVAL_NS  10000000
#define CALC_INTERVAL_NS   10000000

#define INITIAL_REF 0.0

double Kp = 1.5;
double Ki = 0.01;
double Kd = 0.005;

void calc_thread(union sigval s);
void trim(char *str);

int main(int argc, char *argv[]) {
	// Initialize transform structure
	transfer_args_t args;
	args.output = 0;
	args.inputs[0] = 0;
	args.inputs[1] = 0;
	args.inputs[2] = 0;
	args.ref = INITIAL_REF;
	args.out_interval = OUTPUT_INTERVAL_NS;
	args.in_interval = INPUT_INTERVAL_NS;
	args.calc_interval = CALC_INTERVAL_NS;
	args.fd = NULL;
	args.mutex = malloc(sizeof(pthread_mutex_t));
	if(args.mutex == NULL){
		fprintf(stderr, "mutex allocation failed.\n");
		return EXIT_FAILURE;
	}
	pthread_mutex_init(args.mutex, NULL);

	// Give main process root access
	int privityError = ThreadCtl(_NTO_TCTL_IO, NULL);
	if (privityError == -1) {
		fprintf(stderr, "can't get root permissions\n");
		return EXIT_FAILURE;
	}

	pwm_t pwm;
	pwm_init(&pwm, 127);

	init_isr();
	init_encoder();
	init_accelerometer();

	char input[INPUT_BUF_LEN];
	while (1) {
		printf("> ");
		fgets(input, INPUT_BUF_LEN, stdin);
		trim(input);

	}

	return EXIT_SUCCESS;
}

/**
 * Removes trailing line breaks from the string.
 * @param str The string to trim.
 */
void trim(char *str) {
	while (*str != '\n' && *str != '\r' && *str != NULL) str++;
	*str = NULL;
}

/**
 * Thread to perform calculations.
 */
void calc_thread(union sigval s) {
	transfer_args_t *args = s.sival_ptr;

	pthread_mutex_lock(args->mutex);

	// Calculate the next output
	args->output = args->output + (Kp + Ki + Kd)*(args->inputs[0]) - (Kp + 2*Kd)*(args->inputs[1]) + Kd*(args->inputs[2]);
	//printf("ATDin: %lf  out: %lf\n", args->inputs[0], args->output);

	// Write to log file.
	if (args->fd != NULL) {
		struct timespec time;
		clock_gettime(CLOCK_REALTIME, &time);
		fprintf(args->fd, "%ld.%03ld, %lf, %lf\n", (long int)time.tv_sec, time.tv_nsec/1000000, args->inputs[0], args->output);
	}

	pthread_mutex_unlock(args->mutex);
}


