/*
 * BalancingRobot.c
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
#include <signal.h>
#include <time.h>
#include <sys/neutrino.h> // for ThreadCtl()
#include <sys/siginfo.h>
#include "transform.h"
#include "dio.h"
#include "accelerometer.h"
#include "pwm.h"

#define INPUT_BUF_LEN 50

#define OUTPUT_INTERVAL_NS 33334000  // ~30Hz
#define INPUT_INTERVAL_NS  ACCEL_MAX_PERIOD

#define CENTER_TOLERANCE 0.005

#define INITIAL_REF 1.53

static double Kp = 0.00;
static double Ki = 0.00;
static double Kd = 0.00;

static pwm_t pwm;
static motor_t motor;

void calc_thread(union sigval s);
void trim(char *str);

/**
 * SIGTERM handler to shutdown the PWM and motor.
 * @param The number of the signal caught.
 */
void term(int signum){
	if(signum == SIGTERM){
		pwm_set(&pwm, 0);
		motor_free(&motor);
		exit(EXIT_SUCCESS);
	}
}

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

	// Initialize the accelerometer.
	accel_init(&args);

	// Initialize motor controller.
	dio_init();
	pwm_init(&pwm, 0);
	motor_init(&motor, &pwm);
	signal(SIGTERM, term);
	args.motor = &motor;

	// Setup calculation timer.
	struct sigevent event;
	SIGEV_THREAD_INIT(&event, &calc_thread, &args, NULL);

	timer_t timer;
	timer_create(CLOCK_REALTIME, &event, &timer);

	struct timespec millisec = { .tv_sec = 0, .tv_nsec = args.out_interval };
	struct timespec zero = { .tv_sec = 0, .tv_nsec = 0 };
	struct itimerspec time_run = { .it_value = millisec, .it_interval = millisec };
	struct itimerspec time_stop = { .it_value = zero, .it_interval = zero };
	timer_settime(timer, 0, &time_stop, NULL);


	char input[INPUT_BUF_LEN];
	while (1) {
		printf("> ");
		fgets(input, INPUT_BUF_LEN, stdin);
		trim(input);

		if (strncmp(input, "start", INPUT_BUF_LEN) == 0) {
			timer_settime(timer, 0, &time_run, NULL);
		} else if (strncmp(input, "stop", INPUT_BUF_LEN) == 0) {
			timer_settime(timer, 0, &time_stop, NULL);
			pwm_set(&pwm, 0);
			motor_free(&motor);
		} else if (strncmp(input, "quit", INPUT_BUF_LEN) == 0) {
			break;
		} else if (strncmp(input, "setp ", 5) == 0) {
			Kp = atof(input + 5);
		} else if (strncmp(input, "seti ", 5) == 0) {
			Ki = atof(input + 5);
		} else if (strncmp(input, "setd ", 5) == 0) {
			Kd = atof(input + 5);
		} else if (strncmp(input, "setref ", 7) == 0) {
			args.ref = atof(input + 7);
		} else {
			printf("Invalid command (start, stop, setp, seti, setd, setref, quit)\n");
		}
	}

	return EXIT_SUCCESS;
}

/**
 * Removes trailing line breaks from the string.
 * @param str The string to trim.
 */
void trim(char *str) {
	while (*str != '\n' && *str != '\r' && *str != '\0') str++;
	*str = '\0';
}

/**
 * Thread to perform calculations.
 */
void calc_thread(union sigval s) {
	transfer_args_t *args = s.sival_ptr;

	pthread_mutex_lock(args->mutex);

	// Calculate the next output
	args->output = args->output + (Kp + Ki + Kd)*(args->inputs[0]) - (Kp + 2*Kd)*(args->inputs[1]) + Kd*(args->inputs[2]);
	// Correct boundaries and center point.
	if (args->output > 1){
		args->output = 1;
	}else if (args->output < -1){
		args->output = -1;
	}else if(args->inputs[0] < CENTER_TOLERANCE && args->inputs[0] > -CENTER_TOLERANCE){
		args->output = 0;
	}

	printf("ATDin: %lf  out: %lf\n", args->inputs[0], args->output);

	// Write to log file.
	if (args->fd != NULL) {
		struct timespec time;
		clock_gettime(CLOCK_REALTIME, &time);
		fprintf(args->fd, "%ld.%03ld, %lf, %lf\n", (long int)time.tv_sec, time.tv_nsec/1000000, args->inputs[0], args->output);
	}

	pthread_mutex_unlock(args->mutex);

	// Set the motor outputs.
	if (args->output < 0) {
		motor_forward(args->motor, (-args->output)*255);
	} else {
		motor_reverse(args->motor, args->output*255);
	}
}


