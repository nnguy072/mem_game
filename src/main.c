#include<avr/io.h>
#include "timer.h"
#include "scheduler.h"

//y-axis that determine which row that can be lit
//x-axis that actually lights up the column
unsigned char yAxis;
unsigned char xAxis;

// original  x/y value for matrix
// dot/cursor the user moves
unsigned char dot_yVal = 0x04;
unsigned char dot_xVal = 0xEF;

// for the user pattern
unsigned char pat_yVal[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
unsigned char pat_xVal[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// used to iterate through the user pattern
unsigned char i =  0;

//for checking out-of-range
unsigned char upMask = 0x01;
unsigned char downMask = 0x80;
unsigned char leftMask = 0x80;
unsigned char rightMask = 0x01;
unsigned char buttonMask = 0x0F;

// L/R and U/D values for joystick
unsigned short L_R_Val = 0x00;
unsigned short U_D_Val = 0x00;

//---------------------------------------------------------------------------------------
// FROM: http://www.embedds.com/interfacing-analog-joystick-with-avr/
void ADC_init(void)
{
	ADMUX|=(1<<REFS0);
	ADCSRA|=(1<<ADEN)|(1<<ADPS0)|(1<<ADPS1)|(1<<ADPS2); //ENABLE ADC, PRESCALER 128
}

unsigned short readadc(uint8_t ch)
{
	ch&=0b00000111;         //ANDing to limit input to 7
	ADMUX = (ADMUX & 0xf8)|ch;  //Clear last 3 bits of ADMUX, OR with ch
	ADCSRA|=(1<<ADSC);        //START CONVERSION
	while((ADCSRA)&(1<<ADSC));    //WAIT UNTIL CONVERSION IS COMPLETE
	return(ADC);        //RETURN ADC VALUE
}
//------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------
// FROM EECS 120B LAB X: EXTERNAL REGISTERS
void transmit_data(unsigned char data) {
	int i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTC = 0x08;
		// set SER = next bit of data to be sent.
		PORTC |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x02;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTC |= 0x04;
	// clears all lines in preparation of a new transmission
	PORTC = 0x00;
}//------------------------------------------------------------------------------------------

// DRAWS DOT/CURSOR
enum dot_states{dot} dot_state;
void dot_draw()
{
	switch(dot_state)
	{
		case dot:
			break;
			//dot_state = dot;
		default:
			break;
	}
	switch(dot_state)
	{
		case dot:
			yAxis = dot_yVal;
			xAxis = dot_xVal;
			break;
		default:
			break;
	}
	
	
	transmit_data(yAxis);
	PORTD = xAxis;
}

// STATE MACHINE FOR PATTERN USER DRAWS
enum display_states{on, off} display_state;
void display()
{
	switch(display_state)
	{
		case on:
			display_state = off;
			break;
		case off:
			display_state = on;
			break;
	}
	switch(display_state)
	{
		case on:
			transmit_data(pat_yVal[i]);
			PORTD = pat_xVal[i];
			i++;
			if(i == 8)
				i = 0;
			break;	
		case off:
			PORTD = 0xFF;
			break;
	}
}

// STATE MACHINE FOR SHIFTING/MOVING DOT
enum shift_states{start, sshift}shift_state;	
void shift(unsigned short refX, unsigned short refY)
{
	L_R_Val = readadc(0);	// read ADC from PA0 L/R
	U_D_Val = readadc(1);	// READ ADC from PA1 U/D
	
	switch(shift_state)
	{
		case start:
			// check if joystick is moved U/D/L/R
			if((L_R_Val > (refX + 10)) ||
			   (L_R_Val < (refX - 10)) ||
			   (U_D_Val > (refY + 10)) ||
			   (U_D_Val < (refY - 10)) )
				shift_state = sshift;
			break;
		case sshift:
			shift_state = start;
			break;
		default:
			shift_state = start;
	}
	switch(shift_state)
	{
		case start:
			break;
		case sshift:
			// (L_R_Val > (refX + 10))	// joystick right
			// (L_R_Val < (refX - 10))	// joystick left
			// (U_D_Val > (refY + 10))	// joystick up
			// (U_D_Val < (refY - 10))	// joystick down
			
			/*
			// used for "normal" orientation
			if((U_D_Val > (refY + 10)) && ((yVal & upMask) != 0x01))	// check upper boundary 
				yVal = yVal >> 1;	// shift up
			else if((U_D_Val < (refY - 10)) && ((yVal & downMask) != 0x80))	// check lower boundary
				yVal = yVal << 1;	// shift down
			if((L_R_Val < (refX - 10)) && ((xVal & leftMask) != 0x00))	// check left boundary
				xVal = (xVal << 1) | 0x01;	// shift left
			else if((L_R_Val > (refX + 10)) && (xVal & rightMask) != 0x00)	// check right boundary
				xVal = (xVal >> 1) | 0x80;	// shift right
			*/
			
			// rotated orientation because I tilted my breadboard
			if((L_R_Val > (refX + 10)) && ((dot_yVal & upMask) != 0x01))	// check upper boundary
				dot_yVal = dot_yVal >> 1;	// shift up
			else if((L_R_Val < (refX - 10)) && ((dot_yVal & downMask) != 0x80))	// check lower boundary
				dot_yVal = dot_yVal << 1;	// shift down
			if((U_D_Val > (refY + 10)) && ((dot_xVal & leftMask) != 0x00))	// check left boundary
				dot_xVal = (dot_xVal << 1) | 0x01;	// shift left
			else if((U_D_Val < (refY - 10)) && (dot_xVal & rightMask) != 0x00)	// check right boundary
				dot_xVal = (dot_xVal >> 1) | 0x80;	// shift right
			
			break;
	}
}

//enum states{start, press, release, clear} draw_state;


int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	// elapsed time for each state machines
	unsigned long dot_elaspedTime = 1;
	unsigned long shift_elaspedTime = 100;
	unsigned long display_elaspedTime = 1;
	
	const unsigned char period = 1;
	TimerSet(period);
	TimerOn();
	ADC_init();

	unsigned short refX = readadc(0);
	unsigned short refY = readadc(1);
	
	while(1)
	{
		if(dot_elaspedTime >= 10){
			dot_draw();
			dot_elaspedTime = 0;
		}
		if(dot_elaspedTime >= 1){
			display();
			display_elaspedTime = 0;
		}
		if(shift_elaspedTime >= 100)
		{
			shift(refX, refY);
			shift_elaspedTime = 0;
		}
		
		while(!TimerFlag);
		TimerFlag = 0;
		
		dot_elaspedTime += period;
		shift_elaspedTime += period;
		display_elaspedTime += period;
	}
}