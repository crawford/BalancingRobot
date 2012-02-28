/*
 * motor.c
 *
 *  Created on: Feb 19, 2012
 *      Author: Alex Crawford
 *              Conlan Wesson
 */

#include "motor.h"

#include "dio.h"

#define MOTOR_FWD_BIT  0x12
#define MOTOR_RVS_BIT  0x24

/**
 * Initialize the motor.
 * @param motor Motor control structure.
 * @param pwm PWM to use for the motor.
 * @return The motor control structure if successful.
 */
motor_t *motor_init(motor_t *motor, pwm_t *pwm){
	motor->pwm = pwm;

	// Stop the PWM.
	pwm_set(motor->pwm, 0);

	// Clear the controller bits.
	dio_clear(MOTOR_FWD_BIT | MOTOR_RVS_BIT);

	return motor;
}

/**
 * Begin moving the motor forward.
 * @param motor The motor control structure.
 * @param speed The duty cycle of the motor (0-255).
 */
void motor_forward(motor_t *motor, uint8_t speed){
	// Clear the controller bits.
	dio_clear(MOTOR_RVS_BIT);

	pwm_set(motor->pwm, speed);

	// Set forward bit.
	dio_set(MOTOR_FWD_BIT);
}

/**
 * Begin moving the motor backwards.
 * @param motor The motor control structure.
 * @param speed The duty cycle of the motor (0-255).
 */
void motor_reverse(motor_t *motor, uint8_t speed){
	// Clear the controller bits.
	dio_clear(MOTOR_FWD_BIT);

	pwm_set(motor->pwm, speed);

	// Set reverse bit.
	dio_set(MOTOR_RVS_BIT);
}

/**
 * Begin braking the motor.
 * @param motor The motor control structure.
 */
void motor_brake(motor_t *motor){
	// Set both controller bits.
	dio_set(MOTOR_FWD_BIT | MOTOR_RVS_BIT);

	pwm_set(motor->pwm, 255);
}

/**
 * Allow motor to spin freely.
 * @param motor The motor control structure.
 */
void motor_free(motor_t *motor){
	pwm_set(motor->pwm, 0);

	// Clear the controller bits.
	dio_clear(MOTOR_FWD_BIT | MOTOR_RVS_BIT);
}
