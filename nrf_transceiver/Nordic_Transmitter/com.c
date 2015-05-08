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

#define ADC_PRESCALER 0
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define PACKET_SIZE 16

char mode = 't';

void initUART(void){

  UBRR1 = 25;  //38.4k baud
  UCSR1A = (1<<U2X1);
  UCSR1B = (1<<RXEN1) | (1<<TXEN1);
  UCSR1C = (1<<UCSZ11) | (1<<UCSZ10);  //1 stop bit
}
void transmit_payload(uint8_t *data){
 
  CE_HIGH;
  initRadioTX();
  _delay_ms(5);
  for(int i = 0; i < PACKET_SIZE; ++i){
    data[i] = 'a' + i; 
  }
  setRadioTXPayload(data, PACKET_SIZE);
  CE_HIGH;
  _delay_us(15);  // pulse CE to start transmition
  CE_LOW;

  while(!(getTX_DS())){

    _delay_us(300);  //retransmit time determined by SETUP_RETR register

    if(getMAX_RT()){

      clearMAX_RT();
      CE_HIGH;
      _delay_us(15);
      CE_LOW;
    }
  }
  clearTX_DS();
  _delay_ms(5);
  mode = 'r';
}
void receive_payload(uint8_t *data){
 
  CE_HIGH;
  initRadioRX();
  _delay_ms(5);  
  clearRX_DR();
  getRadioRXPayload(data, PACKET_SIZE); //read payload
  CE_LOW;

  for(int i = 0; i < PACKET_SIZE; i++){

    while(!(UCSR1A & (1 << UDRE1)));
    UDR1 = data[i];
  }
  CE_HIGH; 
  _delay_ms(5); 
}
int main(void){

  uint8_t data[PACKET_SIZE] = {0};
  uint8_t *load = (uint8_t *)malloc(PACKET_SIZE*sizeof(uint8_t));

  CPU_PRESCALE(0x01);  // run at 8 MHz
  INIT_CSN;
  INIT_CE;
  CSN_HIGH;
  initUART();
  SPI_masterInit();
  
  initRadioRX();
  setRadioAddressWidth(THREE_BYTES);
  setRadioTXAddress(0xABC123);        
  setRadioRXAddress(0xABC123);
  setRadioFrequency(0b00000010);
  setRadioRXPayloadSize(DATA_PIPE_0, PACKET_SIZE);
  _delay_ms(5);

  while(1){
    uint8_t radioStatus = 0;
    getRadioStatus(&radioStatus);
    while(!(UCSR1A & (1 << UDRE1)));
    UDR1 = 'a';
    if(mode == 'r'){  
      if(getRX_DR()){
        receive_payload(load);
      } 
    }
    if(mode == 't'){
      transmit_payload(data);
    }
  }
  free(load);
}
