// Timer32.h
// Runs on MSP432

// Adapted from SysTick.h from the book:
/* "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1, Program 4.7
*/

#ifndef __TIMERA0_H__
#define __TIMERA0_H__

// Time delay using busy wait
// The delay parameter is in units of the core clock (units of 12.5 nsec for 80 MHz clock)
void TimerA0_Wait( uint32_t delay );

// Time delay using busy wait
// This assumes 80 MHz system clock
void TimerA0_Wait10ms( uint32_t delay );

#endif
