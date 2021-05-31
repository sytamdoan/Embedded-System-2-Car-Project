// CECS347Project1.c
// Runs on LM4F120 or TM4C123

#include <stdint.h>
#include "PLL.h"
#include "PWM.h"
#include "tm4c123gh6pm.h"
#include "Nokia5110.h"

// Constants
#define LIGHT 			GPIO_PORTF_DATA_R
#define redLight    0x02
#define blueLight   0x04
#define greenLight  0x08

#define PERIOD 			40000           	// number of machine cycles for 10ms, value is based on 16MH system clock
#define MAX_DUTY    39200						// maximum duty cycle 98%
#define DUTY_STEP		12000						// duty cycle change for each button press
#define NO_DUTY			0						

// Function prototypes
// External functions for interrupt control defined in startup.s
extern void DisableInterrupts(void); // Disable interrupts
extern void EnableInterrupts(void);  // Enable interrupts
extern void WaitForInterrupt(void);  // low power mode

// This function initilizes port F and arm PF4, PF0 for falling edge interrupts
void Switch_Init(void);
void Delay(void);
void Light_Init(void);
void motors_Init(void);
void DelayLCD(unsigned long ulCount);

// Global variables: 
// H: number of clocks cycles for duty cycle
// direction: Controls the direction of each motor. If direction is 1, then the car moves forward, if it's 0, then it moves backwards.
unsigned long H;
unsigned char direction = 0; //direction 1 means moving forward, 0 means moving backwards


int main(void){

  DisableInterrupts();  // disable interrupts to allow initializations
	PLL_Init(); //initialize PLL at 50MHz
  Switch_Init(); //initialize the switches so they can act as a means to increase or decrease duty cycle
	motors_Init(); //initialize the motors so they can move forward or backwards
	Light_Init();	// arm PF4, PF0 for falling edge interrupts
	Nokia5110_Init();
  EnableInterrupts();   // enable after initializations are done
	
	Nokia5110_Clear();
	Nokia5110_OutString("************CECS347 Lab3************Speed: Dir: ------ ----- ");
	Nokia5110_SetCursor(7, 5);          // seven leading spaces, bottom row
	Nokia5110_OutChar('F');
	Nokia5110_SetCursor(0, 5);          // 0 leading spaces, bottom row
	Nokia5110_OutChar('0');
	Nokia5110_OutChar('%');
	Nokia5110_OutChar(' ');
	Nokia5110_OutChar(' ');
	//The car starts at the stop status with red light on. When the duty cycle is increased, the car will then move forward
	direction = 1;
	LIGHT = redLight;
	
  while(1){
		
		//If the Light is red, then there is no movement in the motor
		if((LIGHT & redLight) == 0x02) {
			PWM0A_Init(PERIOD, 0);
			PWM0B_Init(PERIOD, 0);
			Delay();
		//If the Light is blue, then there is backwards movements in the motor	
		} else if ((LIGHT & blueLight) == 0x04) {
			GPIO_PORTE_DATA_R = 0x0E;
			GPIO_PORTB_DATA_R = 0x01;
			PWM0A_Init(PERIOD, H);
			PWM0B_Init(PERIOD, H);
			Nokia5110_SetCursor(7, 5);          // five leading spaces, bottom row
			Nokia5110_OutChar('B');
			Delay();
	//If the Light is green, then there is forward movements in the motor
		} else if ((LIGHT & greenLight) == 0x08) {
			GPIO_PORTE_DATA_R = 0x0D;
			GPIO_PORTB_DATA_R = 0x02;
			PWM0A_Init(PERIOD, H);
			PWM0B_Init(PERIOD, H);
			Nokia5110_SetCursor(7, 5);          // seven leading spaces, bottom row
			Nokia5110_OutChar('F');
			Delay();
		}
		
  }
}



// Initilize port F and arm PF4, PF0 for falling edge interrupts. Button 1 will be use to increase the duty cycle. Button 2 will change the direction of the motors
void Switch_Init(void){  unsigned long volatile delay;
	H = 0;
  SYSCTL_RCGC2_R |= 0x00000020; // (a) activate clock for port F
  delay = SYSCTL_RCGC2_R;
  GPIO_PORTF_LOCK_R = 0x4C4F434B; // unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x11;         // allow changes to PF4,0
  GPIO_PORTF_DIR_R &= ~0x11;    // (c) make PF4,0 in (built-in button)
  GPIO_PORTF_AFSEL_R &= ~0x11;  //     disable alt funct on PF4,0
  GPIO_PORTF_DEN_R |= 0x11;     //     enable digital I/O on PF4,0
  GPIO_PORTF_PCTL_R &= ~0x000F000F; //  configure PF4,0 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0x11;  //     disable analog functionality on PF4,0
  GPIO_PORTF_PUR_R |= 0x11;     //     enable weak pull-up on PF4,0
  GPIO_PORTF_IS_R &= ~0x11;     // (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11;    //     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11;    //     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;      // (e) clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;      // (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF1FFFFF)|0x00400000; // (g) bits:23-21 for PORTF, set priority to 2
  NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC
}
//Initilize port F PF3-1 for output LED
void Light_Init(void){
	SYSCTL_RCGC2_R |= 0X00000020;
	while ((SYSCTL_RCGC2_R&0x00000020)!=0x00000020){}
	GPIO_PORTF_LOCK_R = 0x4C4F434B;
  GPIO_PORTF_AMSEL_R &= ~0x0E; 
  GPIO_PORTF_PCTL_R &= ~0x0000FFF0; 
  GPIO_PORTF_DIR_R |= 0x0E;   
  GPIO_PORTF_AFSEL_R &= ~0x0E; 
  GPIO_PORTF_DEN_R |= 0x0E;  
	
}
//The outputs of the motor
//PE0 and PE1 is use to control 1 motor, PB0 and PB1 is use to control motor 2
void motors_Init(void){
	SYSCTL_RCGC2_R |= 0X00000010;
	while ((SYSCTL_RCGC2_R&0x00000010)!=0x00000010){}
  GPIO_PORTE_AMSEL_R &= ~0x03; 
  GPIO_PORTE_PCTL_R &= ~0x000000FF; 
  GPIO_PORTE_DIR_R |= 0x03;   
  GPIO_PORTE_AFSEL_R &= ~0x03; 
  GPIO_PORTE_DEN_R |= 0x03;  
		
		
	SYSCTL_RCGC2_R |= 0X00000002;
	while ((SYSCTL_RCGC2_R&0x00000002)!=0x00000002){}
  GPIO_PORTB_AMSEL_R &= ~0x03; 
  GPIO_PORTB_PCTL_R &= ~0x000000FF; 
  GPIO_PORTB_DIR_R |= 0x03;   
  GPIO_PORTB_AFSEL_R &= ~0x03; 
  GPIO_PORTB_DEN_R |= 0x03;
	
}
// PORTF ISR:
// sw1: increase duty cycle depending on current duty cycle. Note that the increase is not linear.
// sw2: Changes the direction of the motor

void GPIOPortF_Handler(void){ // called on touch of either SW1 or SW2
  if(GPIO_PORTF_RIS_R&0x01){  // SW2 touched 
		
		Delay(); //This delay will serve as a debounce for the circuit. The debounce will be around 0.4 second
		GPIO_PORTF_ICR_R = 0x01;  // acknowledge flag1
		
		//When switch 2 is pressed and the light is at red, the direction and light will change when duty cycle is increased, but current light will remain red
		if((LIGHT & redLight) == 0x02){ 
			direction = (direction == 0) ? 1: 0;
			Nokia5110_SetCursor(7, 5);          // seven leading spaces, bottom row
			if(direction == 1) {
				Nokia5110_OutChar('F');
			} else {
				Nokia5110_OutChar('B');
			}
		}
		//When switch 2 is pressed and the light is at green, the direction and light will change to blue
		else if(direction == 1) {
			direction = 0;
			LIGHT = blueLight;
		}
		//When switch 2 is pressed and the light is at blue, the direction and light will change to green
		else if(direction == 0) {
			direction = 1;
			LIGHT = greenLight;
		}
  }
	
  if(GPIO_PORTF_RIS_R&0x10){  // SW1 touch
	
		Delay(); //This delay will serve as a debounce for the circuit. The debounce will be around 0.4 second
    GPIO_PORTF_ICR_R = 0x10;  // acknowledge flag4
		
		
		
		//When button 1 is pressed, the motor will have its duty cycles increased. The increase in duty cycle will not be linear. 
    if(H<MAX_DUTY){ 
			if(H == 0){
				H = H + 12000;
				Nokia5110_SetCursor(0, 5);          // 0 leading spaces, bottom row
				Nokia5110_OutChar('3');
				Nokia5110_OutChar('0');
				Nokia5110_OutChar('%');
				Nokia5110_OutChar(' ');
				Nokia5110_OutChar(' ');
				
			}	else if(H == 12000){
				H = H + 12000;
				Nokia5110_SetCursor(0, 5);          // 0 leading spaces, bottom row
				Nokia5110_OutChar('6');
				Nokia5110_OutChar('0');
				Nokia5110_OutChar('%');
				Nokia5110_OutChar(' ');
				Nokia5110_OutChar(' ');
				
			}	else if (H == 24000) {
				H = 32000;
				Nokia5110_SetCursor(0, 5);          // 0 leading spaces, bottom row
				Nokia5110_OutChar('8');
				Nokia5110_OutChar('0');
				Nokia5110_OutChar('%');
				Nokia5110_OutChar(' ');
				Nokia5110_OutChar(' ');
				
			} else if (H == 32000) {
				H = MAX_DUTY;
				Nokia5110_SetCursor(0, 5);          // 0 leading spaces, bottom row
				Nokia5110_OutChar('9');
				Nokia5110_OutChar('8');
				Nokia5110_OutChar('%');
				Nokia5110_OutChar(' ');
				Nokia5110_OutChar(' ');
			}
			
			//This part of the code also checks for the direction variable. Depending on the value, the light will be green for forward movement or blue for backwards movement
			if(direction == 1) {
				LIGHT = greenLight;
			}
			if(direction == 0) {
				LIGHT = blueLight;
			}
			
		}
		
		//If the Duty cycle is already at max and button 1 is pressed, then the duty cycle will revert back to 0 and the light will turn red
		else if(H==MAX_DUTY){
			H = 0; // increase delivered power
			LIGHT = redLight;
			Nokia5110_SetCursor(0, 5);          // 0 leading spaces, bottom row
			Nokia5110_OutChar('0');
			Nokia5110_OutChar('%');
			Nokia5110_OutChar(' ');
			Nokia5110_OutChar(' ');
		}
  }
}
//This function serves as a debounce of 0.1 seconds
void Delay(void){unsigned long volatile time;
  time = 581792*100/91;  // 0.4	sec
  while(time){
		time--;
  }
	for (time=0;time<1000;time=time+10) {
	}
}

void DelayLCD(unsigned long ulCount){
  do{
    ulCount--;
	}while(ulCount);
}
