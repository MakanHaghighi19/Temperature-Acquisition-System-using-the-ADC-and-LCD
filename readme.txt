EGEC 450 Lab 4 - Temperature Acquisition System using the ADC and LCD

This project implements a real-time temperature measurement system using the Tiva-C microcontroller, an ADC, and an LCD display. The system periodically samples temperature data, converts it to a fixed-point value, and displays it using a custom LCD driver. Developed in C using Code Composer Studio for the EGEC 450 course at California State University, Fullerton.

Table of Contents

- Overview
- Features
- Hardware Requirements
- Build and Run Instructions
- Code Structure
- LCD Driver
- Temperature Acquisition System
- Timer and Interrupt Logic
- Demo Requirements
- References
- License

Overview

The lab is divided into two parts:

1. LCD Driver  
   Implements a low-level driver for a 16x2 Hitachi HD44780 LCD using a 4-bit parallel interface. Provides functions for writing strings, unsigned integers, and fixed-point numbers.

2. Temperature Acquisition System  
   Samples temperature from an analog sensor connected to the ADC. Uses periodic timer interrupts to acquire and update the temperature on the LCD.

Features

- 4-bit LCD driver with support for strings, integers, and fixed-point values
- ADC sampling using SysTick or TimerA at 10 Hz
- Real-time data display using a mailbox mechanism
- Fixed-point output with 0.1°F resolution
- Modular C design using separate source and header files

Hardware Requirements

- Tiva-C LaunchPad (TM4C123GH6PM)
- EduBase-V2 Trainer Board with LM45 or equivalent temperature sensor
- 16x2 character LCD with HD44780 controller
- Breadboard and jumper wires
- Optional: Multimeter for voltage verification

Build and Run Instructions

1. Clone the repository:
   git clone https://github.com/your-username/temperature-acquisition-system.git
   cd temperature-acquisition-system

2. Open the project in Code Composer Studio.

3. Set the target to TM4C123GH6PM.

4. Build and run the project:
   Project > Build All
   Run > Debug

Code Structure

- main.c - Main loop, ADC mailbox, and conversion logic
- LCD.c/.h - LCD driver functions (initialization, output, formatting)
- SysTickInts.c/.h - SysTick interrupt handler for periodic sampling
- ADCSWTrigger.c/.h - ADC initialization and sample collection

LCD Driver

- LCD_Init - Initializes LCD in 4-bit mode
- LCD_OutCmd, LCD_OutChar - Low-level data/command output
- LCD_OutString - Outputs null-terminated string
- LCD_OutUDec - Displays unsigned integer
- LCD_OutUFix - Displays fixed-point number in 0.1°F units

Temperature Acquisition System

- Uses ADC channel 8 (PE5) to read voltage from the LM45 sensor
- Converts ADC value to temperature using a linear equation
- Displays fixed-point result on LCD in Fahrenheit
- Mailbox used to pass data between ISR and foreground logic

Timer and Interrupt Logic

- Timer interrupt fires every 100 ms
- ISR toggles LED, samples ADC, stores value in mailbox, sets flag
- Foreground loop waits on flag, processes and displays data

Demo Requirements

- Show hardware setup with LCD and sensor
- Demonstrate real-time temperature updates
- Alter temperature by warming sensor with hand or breath
- Show name on video and describe behavior
- Keep video under 2 minutes

References

- TM4C123GH6PM Datasheet (Texas Instruments)
- LM45 Temperature Sensor Datasheet
- EGEC 450 Lecture Slides on LCD, ADC, and SysTick
- Lab 4 Manual and example code from Canvas

License

This project was completed as coursework for EGEC 450 at CSU Fullerton.  
Do not reuse, repost, or distribute without instructor permission.

Author: Makan Haghighi
