/*
 * isr.h
 *
 *  Created on: Feb 23, 2012
 *      Author: abc4471
 */

#ifndef ISR_H
#define ISR_H

void init_isr();
int register_isr(void (*callback)(), int interrupt_id);

#define DMA_ISR   3
#define TMR_ISR   2
#define DIO_ISR   1
#define ADC_ISR   0

#endif /* ISR_H */
