#include <stdint.h>
#include "LCD.h"
#include "TimerA0.h"
#include "PLL.h"
#include "ADCSWTrigger.h"
#include "SysTickInts.h"

volatile uint8_t mailboxFlag;
volatile uint32_t ADCvalue;


// Convert ADC value to Fahrenheit and scale by 1000 for fixed-point representation
uint32_t ConvertADCToFahrenheit(uint32_t adcValue) {
    // Calculate temperature in Fahrenheit, scaled by 1000 for fixed-point output
    uint32_t tempF = (((adcValue * 594) / 4095) + 32) * 10;
    return tempF;
}

void main() {
    uint32_t temperatureF;

    PLL_Init(Bus80MHz);          // Set system clock to 80 MHz
    LCD_Init();                  // Initialize LCD
    SysTick_Init(8000000);

    // Display static text "Temp (F): " once
    LCD_OutString("Temp (F): "); // Display "Temp (F): " on the first line

    while(1) {

        while (mailboxFlag == 0);

        uint32_t adcSample = ADCvalue;
        mailboxFlag = 0;

        // Convert the ADC sample to Fahrenheit, scaled by 1000
        temperatureF = ConvertADCToFahrenheit(adcSample);

        // Move cursor to position after "Temp (F): " and update only the number
        LCD_OutCmd(0x8A);        // Adjust 0x8A to fit your display positioning
        LCD_OutUFix(temperatureF); // Display temperature in Fahrenheit with decimal

        // Delay before next update
        TimerA0_Wait10ms(100);   // 1-second delay
    }
}
