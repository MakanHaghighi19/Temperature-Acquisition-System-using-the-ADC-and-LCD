// SysTickInts.c
// Edited to run on Tiva-C
// Use the SysTick timer to request interrupts at a particular period.
// Daniel Valvano, Jonathan Valvano
// June 1, 2015

/* This example accompanies the books
   "Embedded Systems: Introduction to MSP432 Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2015
   Volume 1 Program 9.7

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */

#include <stdint.h>
#include "ADCSWTrigger.h"
#include "tm4c123gh6pm.h"
#include "SysTickInts.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

volatile uint32_t Counts;
volatile uint32_t ADCvalue;   // Mailbox for ADC value
volatile uint8_t mailboxFlag; // Flag to indicate new data in the mailbox
uint32_t wait_per;

// **************SysTick_Init*********************
// Initialize SysTick periodic interrupts
// Input: interrupt period
//        Units of period are 12.5ns (assuming 80 MHz clock)
//        Maximum is 2^24-1
//        Minimum is determined by length of ISR
// Output: none
void SysTick_Init(uint32_t period) {
    long sr = StartCritical();
    wait_per = period;

    // Initialize ADC to sample from PE5 (A8)
    ADC0_InitSWTriggerSeq3_Ch8();

    // Initialize Port F for heartbeat LED on PF2
    SYSCTL_RCGCGPIO_R |= 0x20;           // Enable clock for Port F
    while ((SYSCTL_PRGPIO_R & 0x20) == 0); // Wait until clock is ready
    GPIO_PORTF_DIR_R |= 0x04;            // Set PF2 as output
    GPIO_PORTF_DEN_R |= 0x04;            // Enable digital I/O on PF2

    Counts = 0;
    mailboxFlag = 0;

    NVIC_ST_CTRL_R = 0;                  // Disable SysTick during setup
    NVIC_ST_RELOAD_R = period - 1;       // Set reload value
    NVIC_ST_CURRENT_R = 0;               // Any write to CURRENT clears it
    NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x40000000; // Set priority 2
    NVIC_ST_CTRL_R = 0x07;               // Enable SysTick with interrupts

    EnableInterrupts();
    EndCritical(sr);
}

void SysTick_Handler(void) {
    GPIO_PORTF_DATA_R ^= 0x04;           // Toggle heartbeat LED (PF2)

    ADCvalue = ADC0_InSeq3();            // Sample the ADC
    mailboxFlag = 1;                     // Set flag to indicate new data


    GPIO_PORTF_DATA_R ^= 0x04;           // Toggle heartbeat LED again


}

uint32_t SysTick_Mailbox(void) {
    if (mailboxFlag) {                   // Check if new data is available
        mailboxFlag = 0;                 // Clear flag after reading
        return ADCvalue;                 // Return ADC sample
    }
    return 0;                            // Return 0 if no new data
}

