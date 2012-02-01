/*
 * dac.h
 *
 *  Created on: Jan 20, 2012
 *      Author: Alex Crawford
 *              Conlan Wesson
 */

#ifndef DAC_H_
#define DAC_H_

#include <stdint.h>
#include "transform.h"

void dac_init(transfer_args_t*);
void dac_write(uint16_t);

#endif /* DAC_H_ */
