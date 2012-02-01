/*
 * atd.h
 *
 *  Created on: Jan 20, 2012
 *      Author: Alex Crawford
 *              Conlan Wesson
 */

#ifndef ATD_H_
#define ATD_H_

#include <stdint.h>
#include "transform.h"

void atd_init(transfer_args_t*);
int16_t atd_read();

#endif /* ATD_H_ */
