#include<avr/io.h>
#include "timer.h"
#include "scheduler.h"

//y-axis that determine which row that can be lit
//x-axis that actually lights up the column
unsigned char yAxis;
unsigned char xAxis;

// original  x/y value for matrix
// 04/EF
unsigned char yVal = 0x04;
unsigned char xVal = 0xEF;

//for checking out-of-range
unsigned char upMask = 0x01;
unsigned char downMask = 0x80;
unsigned char leftMask = 0x80;
unsigned char rightMask = 0x01;
unsigned char buttonMask = 0x0F;

unsigned char i =  0;

// L/R and U/D values for joystick
unsigned short L_R_Val = 0x00;
unsigned short U_D_Val = 0x00;

enum states{dot}state;

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
	
void tick()
{
	switch(state)
	{
		case dot:
			break;
			state = dot;
		default:
			break;
	}
	switch(state)
	{
		case dot:
			yAxis = yVal;
			xAxis = xVal;
			break;
		default:
			break;
	}
	PORTC = yVal;
	PORTD = xVal;
}

enum shift_states{start, sshift, release}shift_state;
	
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
			shift_state = release;
			break;
		case release:
			// checks if joystick is not moving
			if(((L_R_Val < (refX + 10)) &&
			(L_R_Val > (refX - 10))) ||
			((U_D_Val < (refY + 10)) &&
			(U_D_Val > (refY - 10))) )
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
			if((U_D_Val > (refY + 10)) && ((yVal & upMask) != 0x01))	// check upper boundary 
				yVal = yVal >> 1;	// shift up
			else if((L_R_Val < (refX - 10)) && ((xVal & leftMask) != 0x00))	// check left boundary
				xVal = (xVal << 1) | 0x01;	// shift left
			else if((U_D_Val < (refY - 10)) && ((yVal & downMask) != 0x80))	// check lower boundary
				yVal = yVal << 1;	// shift down
			else if((L_R_Val > (refX + 10)) && (xVal & rightMask) != 0x00)	// check right boundary
				xVal = (xVal >> 1) | 0x80;	// shift right
			break;
		case release:
			break;
	}
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	TimerSet(100);
	TimerOn();
	ADC_init();

	unsigned short refX = readadc(0);
	unsigned short refY = readadc(1);
	
	while(1)
	{
		tick();
		shift(refX, refY);
		
		while(!TimerFlag);
		TimerFlag = 0;
	}
}