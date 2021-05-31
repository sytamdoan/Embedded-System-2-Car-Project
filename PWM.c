// PWM.c
// Runs on TM4C123
// Use PWM0/PB6 and PWM1/PB7 to generate pulse-width modulated outputs.
// Daniel Valvano
// March 28, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to ARM Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2014
  Program 6.7, section 6.3.2

 Copyright 2014 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
#include "tm4c123gh6pm.h"

#define PWM_0_GENA_ACTCMPAD_ONE 0x000000C0  // Set the output signal to 1
#define PWM_0_GENA_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0
#define PWM_0_GENB_ACTCMPBD_ONE 0x00000C00  // Set the output signal to 1
#define PWM_0_GENB_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0

#define SYSCTL_RCC_USEPWMDIV    0x00100000  // Enable PWM Clock Divisor
#define SYSCTL_RCC_PWMDIV_M     0x000E0000  // PWM Unit Clock Divisor
#define SYSCTL_RCC_PWMDIV_2     0x00000000  // /2


// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PB6 and PB7 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2 
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PA6/M1PWM2
void PWM0A_Init(uint16_t period, uint16_t duty){
  SYSCTL_RCGCPWM_R |= 0x02;             // 1) activate PWM1
  SYSCTL_RCGCGPIO_R |= 0x01;            // 2) activate port A
  while((SYSCTL_PRGPIO_R&0x01) == 0){};
  GPIO_PORTA_CR_R |= 0x40;
	GPIO_PORTA_AFSEL_R |= 0x40;           // enable alt funct on PA6
  GPIO_PORTA_PCTL_R &= ~0x0F000000;     // configure PA6 as PWM1
  GPIO_PORTA_PCTL_R |= 0x05000000;
  GPIO_PORTA_AMSEL_R &= ~0x40;          // disable analog functionality on PF2
  GPIO_PORTA_DEN_R |= 0x40;             // enable digital I/O on PF2
  SYSCTL_RCC_R = 0x00100000 |           // 3) use PWM divider
      (SYSCTL_RCC_R & (~0x000E0000));   //    configure for /2 divider
  PWM1_1_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM1_1_GENA_R = 0xC8;                 // low on LOAD, high on CMPA down
		// PA6 goes low on LOAD               // we have two PWM modules, each module has 4 generators, each generator output 2 PWM signals: A, B
  // PA6 goes high on CMPA down
  PWM1_1_LOAD_R = period - 1;           // 5) cycles needed to count down to 0. PWM Module 1 output 6->PF2, output 6 comes from 6/2=3, generator 3, 6%2=0->output A, 5/2 = generator 2
  PWM1_1_CMPA_R = duty - 1;             // 6) count value when output rises
  PWM1_1_CTL_R |= 0x00000001;           // 7) start PWM1
  PWM1_ENABLE_R |= 0x00000004;          // enable PA6/M1PWM2
}


// Output on PA7/M1PWM3
void PWM0B_Init(uint16_t period, uint16_t duty){
  SYSCTL_RCGCPWM_R |= 0x02;             // 1) activate PWM1
  SYSCTL_RCGCGPIO_R |= 0x01;            // 2) activate port A
  while((SYSCTL_PRGPIO_R&0x01) == 0){};
  GPIO_PORTA_CR_R |= 0x80;
	GPIO_PORTA_AFSEL_R |= 0x80;           // enable alt funct on PA7
  GPIO_PORTA_PCTL_R &= ~0xF0000000;     // configure PA7 as PWM1
  GPIO_PORTA_PCTL_R |= 0x50000000;
  GPIO_PORTA_AMSEL_R &= ~0x80;          // disable analog functionality on PA7
  GPIO_PORTA_DEN_R |= 0x80;             // enable digital I/O on PA7
  SYSCTL_RCC_R = 0x00100000 |           // 3) use PWM divider
      (SYSCTL_RCC_R & (~0x000E0000));   //    configure for /2 divider
  PWM1_1_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM1_1_GENB_R = 0xC08;                 // low on LOAD, high on CMPA down
		// PA6 goes low on LOAD               // we have two PWM modules, each module has 4 generators, each generator output 2 PWM signals: A, B
  // PA6 goes high on CMPA down
  PWM1_1_LOAD_R = period - 1;           // 5) cycles needed to count down to 0. PWM Module 1 output 6->PF2, output 6 comes from 6/2=3, generator 3, 6%2=0->output A, 5/2 = generator 2
  PWM1_1_CMPB_R = duty - 1;             // 6) count value when output rises
  PWM1_1_CTL_R |= 0x00000001;           // 7) start PWM1
  PWM1_ENABLE_R |= 0x00000008;          // enable PA7/M1PWM3
}


