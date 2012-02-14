/*
 * encoder.h
 *
 *  Created on: Feb 14, 2012
 *      Author: abc4471
 */

#ifndef ENCODER_H_
#define ENCODER_H_

#define PORT_LENGTH 1
#define BASE_ADDRESS 0x280
#define DIR_ADDRESS (BASE_ADDRESS+11)
#define PORT_C_ADDRESS (BASE_ADDRESS+10)
#define COUNT_LOW_ADDRESS (BASE_ADDRESS+12)
#define COUNT_HIGH_ADDRESS (BASE_ADDRESS+13)
#define COUNTER_CONTROL_ADDRESS (BASE_ADDRESS+15)
#define DMA_CONTROL_ADDRESS (BASE_ADDRESS+4)
#define IO_INTERRUPT 5

int init_encoder();

#endif /* ENCODER_H_ */
