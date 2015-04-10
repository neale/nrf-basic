/**************************************************
Written By: Tanner Christensen

Program Description:
This is a basic example program for a MCU driving
a Nordic 2.4ghz tranceiver. This function constantly
transmits 8 bytes of data, char values 'a' - 'h'.
**************************************************/




#define F_CPU 8000000

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/cpufunc.h>
#include "spi.h"
#include <stdlib.h>
#include "radioctl.h"
#include "i2c.h"

// Teensy 2.0: LED is active high
#if defined(__AVR_ATmega32U4__) || defined(__AVR_AT90USB1286__)
#define LED_ON		(PORTD |= (1<<6))
#define LED_OFF		(PORTD &= ~(1<<6))

// Teensy 1.0: LED is active low
#else
#define LED_ON	(PORTD &= ~(1<<6))
#define LED_OFF	(PORTD |= (1<<6))
#endif

#define ADC_PRESCALER 0
#define LED_CONFIG	(DDRD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define DIT 80		/* unit time for mors e code */
#define BAUD_RATE 38.4k
#define F_CPU 8000000


#define PACKET_SIZE 8


void initUART(void)
{
	//UBRR1 = (F_CPU / 4/ baud - 1) / 2;
	UBRR1 = 25;  //38.4k baud
	UCSR1A = (1<<U2X1);
	UCSR1B = (1<<RXEN1) | (1<<TXEN1);
	UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);  //1 stop bit
}


int main(void)
{
	

	CPU_PRESCALE(0x01);  // run at 8 MHz

	INIT_CSN;
	INIT_CE;
	CSN_HIGH;

	SPI_masterInit();

	initRadioTX();
	setRadioAddressWidth(THREE_BYTES);
	setRadioTXAddress(0xABC123);        
	setRadioRXAddress(0xABC123);
	setRadioFrequency(0b00000010);

  	uint8_t data[PACKET_SIZE] = {0};



	_delay_ms(5);

	while(1)
	{

		
		for(int i = 0; i < PACKET_SIZE; ++i){
			data[i] = 'a' + i; 
		}



    	setRadioTXPayload(data, PACKET_SIZE);
			CE_HIGH;
			_delay_us(15);  // pulse CE to start transmition
			CE_LOW;

		while(!(getTX_DS()))
		{
			_delay_us(300);  //retransmit time determined by SETUP_RETR register

			if(getMAX_RT())
			{
				clearMAX_RT();
				CE_HIGH;
				_delay_us(15);
				CE_LOW;
			}
		}

		clearTX_DS();

		_delay_ms(5);
	}

	 
}






















