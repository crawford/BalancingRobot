/*
 * isr.h
 *
 *  Created on: Feb 23, 2012
 *      Author: abc4471
 */

#ifndef ISR_H
#define ISR_H

const struct sigevent* _isr_io(void* area, int id);
void init_isr(void (*callback)());

#define MAX_CALLBACKS 4
#define IO_INTERRUPT 5

#endif /* ISR_H */
