/*
 * motor.c
 *
 *  Created on: Feb 19, 2012
 *      Author: cpw9613
 */

#include "motor.h"

#include <stdint.h>       /* for uintptr_t */
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/mman.h>     /* for mmap_device_io() */

#define PORT_LENGTH 1

#define PORT_A_DIR_BIT 0x10
#define MOTOR_FWD_BIT  0x02
#define MOTOR_RVS_BIT  0x04

#define BASE_ADDRESS 0x280
#define DIR_ADDRESS (BASE_ADDRESS+11)
#define PORT_A_ADDRESS (BASE_ADDRESS+8)

motor_t *motor_init(motor_t *motor, pwm_t *pwm){
	motor->pwm = pwm;

	pwm_set(motor->pwm, 0);

	uintptr_t dirHandle = mmap_device_io(PORT_LENGTH, DIR_ADDRESS);
	motor->handle = mmap_device_io(PORT_LENGTH, PORT_A_ADDRESS);
	out8(dirHandle, in8(dirHandle) & ~PORT_A_DIR_BIT);

	out8(motor->handle, in8(motor->handle) & ~(MOTOR_FWD_BIT | MOTOR_RVS_BIT));

	return motor;
}

void motor_forward(motor_t *motor, uint8_t speed){
	pwm_set(motor->pwm, speed);
	out8(motor->handle, in8(motor->handle) & ~(MOTOR_FWD_BIT | MOTOR_RVS_BIT));
	out8(motor->handle, in8(motor->handle) | MOTOR_FWD_BIT);
}

void motor_reverse(motor_t *motor, uint8_t speed){
	pwm_set(motor->pwm, speed);
	out8(motor->handle, in8(motor->handle) & ~(MOTOR_FWD_BIT | MOTOR_RVS_BIT));
	out8(motor->handle, in8(motor->handle) | MOTOR_RVS_BIT);
}

void motor_brake(motor_t *motor){
	pwm_set(motor->pwm, 0);
	out8(motor->handle, in8(motor->handle) | (MOTOR_FWD_BIT | MOTOR_RVS_BIT));
}

void motor_free(motor_t *motor){
	pwm_set(motor->pwm, 0);
	out8(motor->handle, in8(motor->handle) & ~(MOTOR_FWD_BIT | MOTOR_RVS_BIT));
}
