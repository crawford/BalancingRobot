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
#define MOTOR_FWD_BIT  0x12
#define MOTOR_RVS_BIT  0x24

#define BASE_ADDRESS   0x280
#define DIR_ADDRESS    (BASE_ADDRESS+11)
#define PORT_A_ADDRESS (BASE_ADDRESS+8)

/**
 * Initialize the motor.
 * @param motor Motor control structure.
 * @param pwm PWM to use for the motor.
 * @return The motor control structure if successful.
 */
motor_t *motor_init(motor_t *motor, pwm_t *pwm){
	motor->pwm = pwm;

	pwm_set(motor->pwm, 0);

	uintptr_t dirHandle = mmap_device_io(PORT_LENGTH, DIR_ADDRESS);
	motor->handle = mmap_device_io(PORT_LENGTH, PORT_A_ADDRESS);
	out8(dirHandle, in8(dirHandle) & ~PORT_A_DIR_BIT);

	out8(motor->handle, in8(motor->handle) & ~(MOTOR_FWD_BIT | MOTOR_RVS_BIT));

	return motor;
}

/**
 * Begin moving the motor forward.
 * @param motor The motor control structure.
 * @param speed The duty cycle of the motor (0-255).
 */
void motor_forward(motor_t *motor, uint8_t speed){
	pwm_set(motor->pwm, speed);
	out8(motor->handle, in8(motor->handle) & ~(MOTOR_FWD_BIT | MOTOR_RVS_BIT));
	out8(motor->handle, in8(motor->handle) | MOTOR_FWD_BIT);
}

/**
 * Begin moving the motor backwards.
 * @param motor The motor control structure.
 * @param speed The duty cycle of the motor (0-255).
 */
void motor_reverse(motor_t *motor, uint8_t speed){
	pwm_set(motor->pwm, speed);
	out8(motor->handle, in8(motor->handle) & ~(MOTOR_FWD_BIT | MOTOR_RVS_BIT));
	out8(motor->handle, in8(motor->handle) | MOTOR_RVS_BIT);
}

/**
 * Begin braking the motor.
 * @param motor The motor control structure.
 */
void motor_brake(motor_t *motor){
	pwm_set(motor->pwm, 0);
	out8(motor->handle, in8(motor->handle) | (MOTOR_FWD_BIT | MOTOR_RVS_BIT));
}

/**
 * Allow motor to spin freely.
 * @param motor The motor control structure.
 */
void motor_free(motor_t *motor){
	pwm_set(motor->pwm, 0);
	out8(motor->handle, in8(motor->handle) & ~(MOTOR_FWD_BIT | MOTOR_RVS_BIT));
}
