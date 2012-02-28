/*
 * dio.h
 *
 *  Created on: Feb 27, 2012
 *      Author: cpw9613
 */

#ifndef __DIO_H_
#define __DIO_H_

#include <stdint.h>

void dio_init();
void dio_set(uint8_t);
void dio_clear(uint8_t);

#endif /* __DIO_H_ */
