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

// hard coded countdown patterns
unsigned char cnt3 = 0;		// used to iterator through patterns
unsigned char threeTwoOne = 3;
unsigned char three_yPat[3] = {0x2A, 0x2A, 0x3E};
unsigned char three_xPat[3] = {0xD7, 0xEF, 0xF7};

unsigned char two_yPat[3] = {0x3A, 0x2A, 0x2E};
unsigned char two_xPat[3] = {0xDF, 0xEF, 0xF7};

unsigned char one_yPat[3] = {0x22, 0x3E, 0x20};
unsigned char one_xPat[3] = {0xDF, 0xEF, 0xF7};

// win screen pattern
unsigned char win_yPat[8] = {0x08, 0x10, 0x22, 0x28, 0x28, 0x22, 0x10, 0x08};
unsigned char win_xPat[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};
	
// hard coded X and O
unsigned char cnt6 = 0;
unsigned char o_yPat[6] = {0x18, 0x24, 0x42, 0x42, 0x24 ,0x18};
unsigned char o_xPat[6] = {0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD};
	
unsigned char x_yPat[6] = {0x44, 0x28, 0x10, 0x10, 0x28, 0x44};
unsigned char x_xPat[6] = {0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD};

// hard coded patterns
unsigned char cnt8 = 0;				// iterate through 
unsigned char levels = 0;			// 3 levels/patterns

// "Hi"
unsigned char hi_yPat[8] = {0x00, 0x3E, 0x08, 0x3E, 0x00, 0x34, 0x00, 0x00};
unsigned char hi_xPat[8] = {0xFF, 0xBF, 0xDF, 0xEF, 0xFF, 0xFB, 0xFF, 0xFF};

// SQUARE
unsigned char pat1_yPat[8] = {0x00, 0x00, 0x3C, 0x24, 0x24, 0x3C, 0x00, 0x00};
unsigned char pat1_xPat[8] = {0xFF, 0xFF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFF, 0xFF};
	
// WEIRD PENTAGON
unsigned char pat2_yPat[8] = {0x1F, 0x21, 0x45, 0x91, 0x91, 0x45, 0x21, 0x1F};
unsigned char pat2_xPat[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};

// used to iterate through the user pattern
unsigned char i =  0;
unsigned char row = 0;

//for checking out-of-range
unsigned char upMask = 0x01;
unsigned char downMask = 0x80;
unsigned char leftMask = 0x80;
unsigned char rightMask = 0x01;

// L/R and U/D values for joystick
unsigned short L_R_Val = 0x00;
unsigned short U_D_Val = 0x00;

// mic
unsigned short mic_val = 0x00;
unsigned short mic_ref = 0x00;

// buttons and flags
unsigned char playing = 0;			// turn game off/on
unsigned char correctPat = 0;		// checks if pattern is correct
unsigned char complete_game = 0;	// see if the user finished all patterns

// TIMERS
unsigned int countdown_cnt = 0;				// counter for countdown
unsigned int countdown_time = 1000;			// set time to 1500ms

unsigned int disp_cnt = 0;					// counter for pattern to replicate
unsigned int disp_pat_time = 2000;			// set time to 2000ms for displaying pattern

unsigned int wait_cnt = 0;					// counter for how long players have
unsigned int wait_time = 10000;				// set time to 10000 for user drawing

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


// Playing state machine
enum play_states{game_start, disp_3, disp_2, disp_1, disp_pat, game_wait, check, fail, pass, complete} game_state;
void play_game()
{
	unsigned char play_button = ~PINA & 0x20;
	
	switch(game_state)
	{
		case game_start:
			if(play_button)
				game_state = disp_3;
			else
				game_state = game_start;
			break;
		case disp_3:
			if(countdown_cnt >= countdown_time){
				countdown_cnt = 0;
				game_state = disp_2;
			}
			else{
				countdown_cnt++;
				game_state = disp_3;
			}
			break;
		case disp_2:
			if(countdown_cnt >= countdown_time){
				countdown_cnt = 0;
				game_state = disp_1;
			}
			else{
				countdown_cnt++;
				game_state = disp_2;
			}
			break;	
		case disp_1:
			if(countdown_cnt >= countdown_time){
				countdown_cnt = 0;
				game_state = disp_pat;
			}
			else{
				countdown_cnt++;
				game_state = disp_1;
			}
			break;
		case disp_pat:
			if(disp_cnt >= disp_pat_time){
				disp_cnt = 0;
				if(levels == 0)
					wait_time = 7000;
				else if(levels == 1)
					wait_time = 10000;
				else if(levels == 2)
					wait_time = 13000;
				game_state = game_wait;
			}
			else{
				disp_cnt++;
				game_state = disp_pat;
			}
			break;
		case game_wait:
			if(play_button)
				game_state = check;
			if(wait_cnt >= wait_time){
				wait_cnt = 0;
				game_state = check;
			}
			else{
				wait_cnt++;
				game_state = game_wait;
			}
			break;
		case check:
			if(correctPat){
				levels++;
				game_state = pass;
			}
			else{
				levels = 0;
				game_state = fail;
			}
			break;
		case pass:
			if(countdown_cnt >= disp_pat_time){
				countdown_cnt = 0;
				if(complete_game)
					game_state = complete;
				else
					game_state = disp_pat;
			}
			else{
				countdown_cnt++;
				game_state = pass;
			}
			break;
		case fail:
			if(countdown_cnt >= countdown_time){
				countdown_cnt = 0;
				game_state = game_start;
			}
			else{
				game_state = fail;
				countdown_cnt++;
			}
			break;
		case complete:
			if(countdown_cnt >= disp_pat_time){
				countdown_cnt = 0;
				if(play_button)
					game_state = game_start;
			}
			else{
				countdown_cnt++;
				game_state = complete;
			}
			break;
		default:
			game_state = game_start;
			break;		
	}
	switch(game_state)
	{
		case game_start:
			playing = 0;
			levels = 0;
			transmit_data(0x00);
			PORTD = 0xFF;
			break;
		case disp_3:
			transmit_data(three_yPat[cnt3]);
			PORTD = three_xPat[cnt3];
			cnt3++;
			if(cnt3 == 3)
				cnt3 = 0;
			break;
		case disp_2:
			transmit_data(two_yPat[cnt3]);
			PORTD = two_xPat[cnt3];
			cnt3++;
			if(cnt3 == 3)
				cnt3 = 0;
			break;
		case disp_1:
			transmit_data(one_yPat[cnt3]);
			PORTD = one_xPat[cnt3];
			cnt3++;
			if(cnt3 == 3)
				cnt3 = 0;
			break;
		case disp_pat:
			if(levels == 0){
				transmit_data(pat1_yPat[cnt8]);
				PORTD = pat1_xPat[cnt8];
				cnt8++;
				if(cnt8 == 8)
					cnt8 = 0;
			}
			else if(levels == 1){
				transmit_data(hi_yPat[cnt8]);
				PORTD = hi_xPat[cnt8];
				cnt8++;
				if(cnt8 == 8)
					cnt8 = 0;
			}
			else if(levels == 2){
				transmit_data(pat2_yPat[cnt8]);
				PORTD = pat2_xPat[cnt8];
				cnt8++;
				if(cnt8 == 8)
					cnt8 = 0;
			}
			break;
		case game_wait:
			playing = 1;
			break;
		case check:
			playing = 0;
			unsigned char checkFlag = 0;
			
			if(levels == 0){
				for(int m = 0; m < 8; m++){
					if(pat_xVal[m] == pat1_xPat[m])
						checkFlag = 1;
					else
						checkFlag = 0;
					if(checkFlag == 0)
						break;
				}
				if(checkFlag){
					for(int m = 0; m < 8; m++){
						if(pat_yVal[m] == pat1_yPat[m])
							checkFlag = 1;
						else
							checkFlag = 0;
						if(checkFlag == 0)
							break;
					}
				}
				correctPat = checkFlag;
			}
			else if(levels == 1){
				for(int m = 0; m < 8; m++){
					if(pat_xVal[m] == hi_xPat[m])
						checkFlag = 1;
					else
						checkFlag = 0;
					if(checkFlag == 0)
						break;
				}
				if(checkFlag){
					for(int m = 0; m < 8; m++){
						if(pat_yVal[m] == hi_yPat[m])
							checkFlag = 1;
						else
							checkFlag = 0;
						if(checkFlag == 0)
							break;
					}
				}
				correctPat = checkFlag;
			}
			else if(levels == 2){
				for(int m = 0; m < 8; m++){
					if(pat_xVal[m] == pat2_xPat[m])
						checkFlag = 1;
					else
						checkFlag = 0;
					if(checkFlag == 0)
						break;
				}
				if(checkFlag){
					for(int m = 0; m < 8; m++){
						if(pat_yVal[m] == pat2_yPat[m])
							checkFlag = 1;
						else
							checkFlag = 0;
						if(checkFlag == 0)
							break;
					}
				}
				correctPat = checkFlag;
			}
			
			
			// clear arrays
			for(int h = 0; h < 8; h++){
				pat_yVal[h] = 0x00;
				pat_xVal[h] = 0xFF;
			}
			break;
		case pass:
			transmit_data(o_yPat[cnt6]);
			PORTD = o_xPat[cnt6];
			cnt6++;
			if(cnt6 == 6)
				cnt6 = 0;
			if(levels == 3){
				levels = 0;
				complete_game = 1;
			}
			break;
		case fail:
			transmit_data(x_yPat[cnt6]);
			PORTD = x_xPat[cnt6];
			cnt6++;
			if(cnt6 == 6)
				cnt6 = 0;
			break;
		case complete:
			transmit_data(win_yPat[cnt8]);
			PORTD = win_xPat[cnt8];
			cnt8++;
			if(cnt8 == 8)
				cnt8 = 0;
			break;
	}
}

// DRAWS DOT/CURSOR
enum dot_states{dot} dot_state;
void dot_draw()
{
	switch(dot_state)
	{
		case dot:
			break;
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
		default:
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
			//using switching L/R for U/D because of orientation of board
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

// draws users patterns
enum draw_states{wait, decide, press, release, clear} draw_state;
void draw_usr_patterns()
{
	// get PA6 & PA7
	unsigned char button = (~PINA) & 0xC0;
	switch(draw_state)
	{
		case wait:
			// waits for button press
			if(button != 0x00)
				draw_state = decide;
			break;
		case decide:
			if(button == 0x80)
				draw_state = press;
			else if(button == 0x40)
				draw_state = clear;
			else if(button == 0xC0)
				draw_state = clear;
			else
				draw_state = wait;
			break;
		case press:
			// PA7 still pressed
			if(button == 0x80)
				draw_state = press;
			// PA6 is pressed
			else if((button == 0x40) || (button == 0xC0))
				draw_state = clear;
			// PA7 is released
			else if(!button)
				draw_state = release;
			break;
		case release:
			draw_state = wait;
			break;
		case clear:
			draw_state = wait;
			break;
		default:
			draw_state = wait;
			break;
	}
	switch(draw_state)
	{
		case wait:
			break;
		case press:
			// draw the dot
			if(dot_xVal == 0x7F){
				pat_xVal[0] = dot_xVal;
				row = 0;
			}
			else if(dot_xVal == 0xBF){
				pat_xVal[1] = dot_xVal;
				row = 1;
			}
			else if(dot_xVal == 0xDF){
				pat_xVal[2] = dot_xVal;
				row = 2;
			}
			else if(dot_xVal == 0xEF){
				pat_xVal[3] = dot_xVal;
				row = 3;
			}
			else if(dot_xVal == 0xF7){
				pat_xVal[4] = dot_xVal;
				row = 4;
			}
			else if(dot_xVal == 0xFB){
				pat_xVal[5] = dot_xVal;
				row = 5;
			}
			else if(dot_xVal == 0xFD){
				pat_xVal[6] = dot_xVal;
				row = 6;
			}
			else if(dot_xVal == 0xFE){
				pat_xVal[7] = dot_xVal;
				row = 7;
			}	
			
			pat_yVal[row] = pat_yVal[row] | dot_yVal;
			break;
		case release:
			break;
		case clear:
			// clear the array
			for(int h = 0; h < 8; h++){
				pat_yVal[h] = 0x00;
				pat_xVal[h] = 0xFF;
			}
			break;
		default:
			break;
	}
}

int main(void)
{
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	// elapsed time for each state machines
	unsigned long dot_elaspedTime = 1;
	unsigned long shift_elaspedTime = 100;
	unsigned long display_elaspedTime = 1;
	unsigned long draw_user_patterns_elaspedTime = 1;
	unsigned long play_elaspedTime = 1;
	
	const unsigned char period = 1;
	TimerSet(period);
	TimerOn();
	ADC_init();

	unsigned short refX = readadc(0);
	unsigned short refY = readadc(1);
	mic_ref = readadc(2);
	
	while(1)
	{
		if(play_elaspedTime >= 1)
		{
			play_game();
			play_elaspedTime = 0;
		}
		if(playing)
		{
			if(display_elaspedTime >= 1){
				display();
				display_elaspedTime = 0;
			}
			if(draw_user_patterns_elaspedTime >= 1){
				draw_usr_patterns();
				draw_user_patterns_elaspedTime = 0;
			}
			if(dot_elaspedTime >= 10){
				dot_draw();
				dot_elaspedTime = 0;
			}
			if(shift_elaspedTime >= 100)
			{
				shift(refX, refY);
				shift_elaspedTime = 0;
			}
		}
		
		while(!TimerFlag);
		TimerFlag = 0;
		
		dot_elaspedTime += period;
		shift_elaspedTime += period;
		display_elaspedTime += period;
		draw_user_patterns_elaspedTime += period;
		play_elaspedTime += period;
	}
}