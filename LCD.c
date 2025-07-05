/*
  size is 1*16
  if do not need to read busy, then you can tie R/W=ground
  ground = pin 1    Vss
  power  = pin 2    Vdd   +3.3V or +5V (VBUS) depending on the device
  ground = pin 3    Vlc   grounded for highest contrast
  PE0   = pin 4    RS    (1 for data, 0 for control/status)
  ground = pin 5    R/W   (1 for read, 0 for write)
  PC6   = pin 6    E     (enable)
  PA2   = pin 11   DB4   (4-bit data)
  PA3   = pin 12   DB5
  PA4   = pin 13   DB6
  PA5   = pin 14   DB7
16 characters are configured as 1 row of 16
addr  00 01 02 03 04 05 ... 0F
*/

#include <stdint.h>
#include "LCD.h"
#include "TimerA0.h"
#include "tm4c123gh6pm.h"

void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode
void LCD_OutChar(char data);

// Macros
#define BusFreq 80					// assuming a 80 MHz system clock
#define T6us 6*BusFreq				// 6us
#define T40us 40*BusFreq			// 40us
#define T160us 160*BusFreq			// 160us
#define T1ms 1000*BusFreq			// 1ms
#define T1600us 1600*BusFreq		// 1.60ms
#define T5ms 5000*BusFreq			// 5ms
#define T15ms 15000*BusFreq			// 15ms

// Global Vars
uint8_t LCD_RS, LCD_E;				// LCD Enable and Register Select

/**************** Private Functions ****************/

void Out_RS_E() {
    GPIO_PORTE_DATA_R = (GPIO_PORTE_DATA_R & ~0x01) | LCD_RS;
    GPIO_PORTC_DATA_R = (GPIO_PORTC_DATA_R & ~0x40) | (LCD_E<<6);
}

void SendPulse() {
	Out_RS_E();
	TimerA0_Wait(T6us);				// wait 6us
	LCD_E = 1;						// E=1, R/W=0, RS=1
	Out_RS_E();
	TimerA0_Wait(T6us);				// wait 6us
	LCD_E = 0;						// E=0, R/W=0, RS=1
	Out_RS_E();
}

void SendChar() {
	LCD_E = 0;
	LCD_RS = 1;						// E=0, R/W=0, RS=1
	SendPulse();
	TimerA0_Wait(T1600us);			// wait 1.6ms
}

void SendCmd() {
	LCD_E = 0;
	LCD_RS = 0;						// E=0, R/W=0, RS=0
	SendPulse();
	TimerA0_Wait(T40us);			// wait 40us
}

/**************** Public Functions ****************/
// Clear the LCD
// Inputs: none
// Outputs: none
void LCD_Clear() {
	LCD_OutCmd(0x01);				// Clear Display
	LCD_OutCmd(0x80);				// Move cursor back to 1st position
}

// Initialize LCD
// Inputs: none
// Outputs: none
void LCD_Init() {
    SYSCTL_RCGC2_R |= 0x00000015;  // 1) activate clock for Ports A, C, and E
    while((SYSCTL_PRGPIO_R&0x015) != 0x015){};// ready?
    GPIO_PORTA_AMSEL_R &= ~0x3C;   // 3) disable analog function on PA5-2
    GPIO_PORTC_AMSEL_R &= ~0x40;   //    disable analog function on PC6
    GPIO_PORTE_AMSEL_R &= ~0x01;   //    disable analog function on PE0
    GPIO_PORTA_PCTL_R &= ~0x00FFFF00;   // 4) configure PA5-2 as GPIO
    GPIO_PORTC_PCTL_R &= ~0x0F000000;   //    configure PC6 as GPIO
    GPIO_PORTE_PCTL_R &= ~0x0000000F;   //    configure PE0 as GPIO
    GPIO_PORTA_DIR_R |= 0x3C;      // 5) set direction register
    GPIO_PORTC_DIR_R |= 0x40;
    GPIO_PORTE_DIR_R |= 0x01;
    GPIO_PORTA_AFSEL_R &= ~0x3C;   // 6) regular port function
    GPIO_PORTC_AFSEL_R &= ~0x40;
    GPIO_PORTE_AFSEL_R &= ~0x01;
    GPIO_PORTA_DEN_R |= 0x3C;      // 7) enable digital port
    GPIO_PORTC_DEN_R |= 0x40;
    GPIO_PORTE_DEN_R |= 0x01;
    GPIO_PORTA_DR8R_R |= 0x3C;      // enable 8 mA drive
    GPIO_PORTC_DR8R_R |= 0x40;
    GPIO_PORTE_DR8R_R |= 0x01;

    LCD_E = 0;
    LCD_RS = 1;
    Out_RS_E();
    TimerA0_Wait(T15ms);  // Wait >15 ms after power is applied
    LCD_OutCmd(0x30);         // command 0x30 = Wake up
    TimerA0_Wait(T5ms);   // must wait 5ms, busy flag not available
    LCD_OutCmd(0x30);         // command 0x30 = Wake up #2
    TimerA0_Wait(T160us); // must wait 160us, busy flag not available
    LCD_OutCmd(0x30);         // command 0x30 = Wake up #3
    TimerA0_Wait(T160us); // must wait 160us, busy flag not available
    LCD_OutCmd(0x28);         // Function set: 4-bit/2-line
    LCD_Clear();
    LCD_OutCmd(0x10);         // Set cursor
    //LCD_OutCmd(0x0C);         // Display ON; Cursor ON
    LCD_OutCmd(0x06);         // Entry mode set
}

// Output a character to the LCD
// Inputs: letter is ASCII character, 0 to 0x7F
// Outputs: none
void LCD_OutChar(char letter) {
	unsigned char let_low = (0x0F&letter)<<2;
	unsigned char let_high = (letter>>2)&0x3C;
    long intstatus = StartCritical();

	GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R & ~0x3C) | let_high;
	SendChar();
    GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R & ~0x3C) | let_low;
	SendChar();
	EndCritical( intstatus );
	TimerA0_Wait(T1ms);				// wait 1ms
}

// Output a command to the LCD
// Inputs: 8-bit command
// Outputs: none
void LCD_OutCmd(unsigned char command) {
	unsigned char com_low = (0x0F&command)<<2;
	unsigned char com_high = (command>>2)&0x3C;
    long intstatus = StartCritical();

    GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R & ~0x3C) | com_high;
	SendCmd();
    GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R & ~0x3C) | com_low;
	SendCmd();
	EndCritical( intstatus );
	TimerA0_Wait(T1ms);				// wait 1ms
}

//------------LCD_OutString------------
// Output String (NULL termination)
// Input: pointer to a NULL-terminated string to be transferred
// Output: none
void LCD_OutString(char *ptr) {
    while (*ptr != '\0') {     // Loop until the null terminator
        LCD_OutChar(*ptr);     // Output the current character to the LCD
        ptr++;                 // Move to the next character in the string
    }
}

//-----------------------LCD_OutUDec-----------------------
// Output a 32-bit number in unsigned decimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1-10 digits with no space before or after
void LCD_OutUDec(uint32_t n) {
    // Base case: if n is less than 10, we can directly output it as a single digit
    if (n < 10) {
        LCD_OutChar(n + '0');  // Convert the digit to ASCII and display it
    } else {
        // Recursive case: output the higher-order digits first
        LCD_OutUDec(n / 10);   // Recursive call with the quotient
        LCD_OutChar((n % 10) + '0'); // Output the last digit
    }
}

//-----------------------LCD_OutUHex-----------------------
// Output a 32-bit number in unsigned hexadecimal format
// Input: 32-bit number to be transferred
// Output: none
// Variable format 1 to 8 digits with no space before or after
void LCD_OutUHex(uint32_t number) {
    // Base case: if number is less than 16, output the single hex digit
    if (number < 16) {
        // If number < 10, output as '0'-'9'; if >=10, output as 'A'-'F'
        if (number < 10) {
            LCD_OutChar(number + '0');
        } else {
            LCD_OutChar((number - 10) + 'A');
        }
    } else {
        // Recursive case: output higher-order hex digits first
        LCD_OutUHex(number / 16);   // Recursive call with the quotient
        uint8_t hex_digit = number % 16; // Last hex digit
        // Output the current hex digit based on its value (0-9 or A-F)
        if (hex_digit < 10) {
            LCD_OutChar(hex_digit + '0');
        } else {
            LCD_OutChar((hex_digit - 10) + 'A');
        }
    }
}

void LCD_OutUFix(uint32_t number) {
    // Check if the number is out of range for the 999.9 format
    if (number > 9999) {
        // Output the out-of-range indicator
        LCD_OutChar('*');
        LCD_OutChar('*');
        LCD_OutChar('*');
        LCD_OutChar('.');
        LCD_OutChar('*');
        LCD_OutChar(' ');
    } else {
        // Calculate the integer and fractional parts
        uint32_t integer_part = number / 10;       // Get the integer part (0-999)
        uint32_t fractional_part = number % 10;    // Get the fractional part (0-9)

        // Output integer part (up to 3 digits)
        if (integer_part >= 100) {   // Hundreds place
            LCD_OutChar((integer_part / 100) + '0');
        }
        if (integer_part >= 10) {    // Tens place
            LCD_OutChar(((integer_part / 10) % 10) + '0');
        }
        LCD_OutChar((integer_part % 10) + '0');    // Ones place

        // Output the decimal point and fractional part
        LCD_OutChar('.');
        LCD_OutChar(fractional_part + '0');

        // Output a space at the end
        LCD_OutChar(' ');
    }
}

