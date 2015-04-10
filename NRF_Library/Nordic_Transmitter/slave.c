#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "uart.h"
#include <avr/cpufunc.h>
#include "usb_debug_only.h"
#include "print.h"
#include "spi.h"

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
#define DIT 80		/* unit time for morse code */
#define BAUD_RATE 9600
#define F_CPU 8000000

void usart_transmit(unsigned char data)
{
	while(!(UCSR1A & (1<<UDRE1)))
		;
	UDR1 = data;
	_delay_ms(100);
}

int main(void)
{
    UBRR1 = 207;//(F_CPU / (8*BAUD_RATE))-1;
    UCSR1A = (1<<U2X1);
    UCSR1B = (1<<RXEN1) | (1<<TXEN1);
    UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);

    DDRD |= 1<<3;

	char recieved = 'a';
    SPI_slaveInit();
	
	while (1) {
		//recieved = slaveRecieve();
		usart_transmit(recieved);
		usart_transmit(' ');
    }
}
