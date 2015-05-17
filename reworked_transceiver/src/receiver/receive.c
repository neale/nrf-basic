#define F_CPU 8000000

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/cpufunc.h>
#include "../../lib/spi/spi.h"
#include <stdlib.h>
#include "../../lib/radio_control/radioctl.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define PACKET_SIZE 8

char mode = 'r';

void initUART(void)
{
  //UBRR1 = (F_CPU / 4/ baud - 1) / 2;
  UBRR1 = 25;  //38.4k baud
  UCSR1A = (1<<U2X1);
  UCSR1B = (1<<RXEN1) | (1<<TXEN1);
  UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);  //1 stop bit
}

void build_packet(uint8_t * data, int size){

  for(int i = 0; i < PACKET_SIZE; ++i){
    data[i] = 'a' + i; 
  } 
}

void uart_putstring(char * s){
  
  int i = 0; 
  while(s[i] != '\0'){
    uart_putchar(s[i]);
    i++;
  }
}

void uart_putchar(char c){

  while (!(UCSR1A & (1 << UDRE1)));
  UDR1 = c;
}


void transmit(uint8_t * data){
 
  SPI_masterInit();
  initRadioTX();
  _delay_ms(8);
  uart_putchar(mode);
  build_packet(data, PACKET_SIZE);  
  if(data[0] == 'a'){
    setRadioTXPayload(data, PACKET_SIZE);
    CE_HIGH;
    _delay_us(15);  // pulse CE to start transmition
    CE_LOW;
    while(!(getTX_DS())){
      uart_putstring("waiting for ack");
      _delay_us(300);  //retransmit time determined by SETUP_RETR register

      if(getMAX_RT()){
        
        clearMAX_RT();
        CE_HIGH;
        _delay_us(15);
        CE_LOW;
      }
      clearTX_DS();
      _delay_ms(5);
    }
    SPI_masterInit();
    initRadioRX();
    _delay_ms(8); 
    uart_putstring("switching to receive");
    mode = 'r';
  }
}

int main(void)
{

  uint8_t *load = (uint8_t *)malloc(PACKET_SIZE*sizeof(uint8_t));


  CPU_PRESCALE(0x01);  // run at 8 MHz

  INIT_CSN;
  INIT_CE;
  CSN_HIGH;
  CE_HIGH;


  initUART();

  // Put receiver in sleep mode

  SPI_masterInit();
  initRadioRX();

  setRadioAddressWidth(THREE_BYTES);
  setRadioRXAddress(0xABC123);                    
  setRadioRXPayloadSize(DATA_PIPE_0, PACKET_SIZE);
  setRadioFrequency(0b00000010); 

  _delay_ms(5);


  while (1) {

    uint8_t radioStatus = 0;
    
    getRadioStatus(&radioStatus);
    if(getRX_DR())  // if RX data received
    {
      clearRX_DR();
      getRadioRXPayload(load, PACKET_SIZE); //read payload
      CE_LOW;
      for(int i = 0; i<PACKET_SIZE; i++)
      {
        while(!(UCSR1A & (1 << UDRE1)));
        UDR1 = load[i];
      }
    }

    CE_HIGH; 
    _delay_ms(5);
  }

  free(load);

}
