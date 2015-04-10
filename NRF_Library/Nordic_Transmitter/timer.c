/*
 * timer.c
 *
 * Created: 7/16/2014 4:31:15 PM
 *  Author: Tanner Christensen
 */ 


#include <avr/io.h>
#include <stdlib.h>
#include "timer.h"



#define F_CPU 8000000



void setupTimer() {
	
	TCCR1A = 0b00000000; //Normal pin operation
	TCCR1B = 0b00001101; //  set TIMER1 clock to clk/1024, CTC mode
	TCNT1 = 0x0000; //initialize TIMER1 to 0
	TIMSK1 |= (1 << OCIE1A) | (1 << TOIE1); //Enable Timer/Counter, and compare interrupts
	OCR1A = 0x3E8; // sets initial TIMER1 COMPARE A to 1000

}


