#define F_CPU 8000000

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>
#include <avr/cpufunc.h>
#include "../../lib/spi/spi.h"
#include <stdlib.h>
#include <assert.h>
#include "../../lib/radio_control/radioctl.h"
#include "../../lib/i2c/i2c.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))
#define F_CPU 8000000

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

/* FIX ME */

//change to look like the collab packet
void build_packet(uint8_t * data){

  for(int i = 0; i < PACKET_SIZE; ++i){
    data[i] = 'a' + i; 
  }
  return data; 
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
/* this is a don question. I have no clue what's happening
 * it works. when the switch is on it sends, off it doesn't
 * and you can flip flop between modes
 * But when you turn off the switch there is about a 10 second
 * delay for it to actually stop transmitting
 * I don't understand why, the program completely ignores my conditions
 * for entering transmission for 10 seconds. 
 *
 * There is some UART / embedded thing going on that I don't 
 * understand
 */
void transmit(uint8_t * data){

  SPI_masterInit();
  initRadioTX(); //turn on transmit mode
  _delay_ms(8); //let it sink in
  uart_putchar(mode); 
  
  /* WRITE THIS */
  build_packet(data); //populate packet with collab packet 
  /*            */
  
  if(!(PINF & (1 << 0))) //dont enter transmission unless switch off
    goto exit; 
  setRadioTXPayload(data, PACKET_SIZE); //load packet for entry
  CE_HIGH;
  _delay_us(15);  // pulse CE to start transmition
  CE_LOW;
  if(!(PINF & (1 << 0)))  //again don't stupid get this far unless switch is still on
    goto exit; 

  /* here is the magic box */
  /* under all conditions this should not be executed */
  while(!(getTX_DS())){
    if(!(PINF & (1 << 0)))
      goto exit;
    if(PINF & (1 << 0))
      goto exit;
    uart_putstring("waiting for ack");
    _delay_us(300);  //retransmit time determined by SETUP_RETR register

    if(getMAX_RT()){

      clearMAX_RT();
      CE_HIGH;
      _delay_us(15);
        CE_LOW;
    }
  }
  /* jump here to exit function */
  exit:
    clearTX_DS();
    _delay_ms(5);

    SPI_masterInit(); //set into receive mode 
    initRadioRX();
    _delay_ms(8);
    uart_putstring("switching to receive");
    mode = 'r';
}
void init_ports(void){

  DDRF &= ~(1 << 0);
  PORTF &= ~(1 << 0);
  EICRA = (1 << ISC00) | (1 << ISC01);
  EIMSK = (1 << INT0);
}

int main(void){

  uint8_t *load = (uint8_t *)malloc(PACKET_SIZE*sizeof(uint8_t));

  CPU_PRESCALE(0x01);  // run at 8 MHz
/* iniitalization, don't touch */
/*=====================================*/
  INIT_CSN;
  INIT_CE;
  CSN_HIGH;
  initUART();
  init_ports();
  SPI_masterInit();
  initRadioTX();

  setRadioAddressWidth(THREE_BYTES);
  setRadioTXAddress(0xABC123);        
  setRadioRXAddress(0xABC123);
  setRadioRXPayloadSize(DATA_PIPE_0, PACKET_SIZE);
  setRadioFrequency(0b00000010);

  uint8_t data[PACKET_SIZE] = {0};

  sei();
  _delay_ms(5);
/* ======================================*/
  
  while(1){
    _delay_ms(100); //woah there buddy slow down
    /* set modes and make decision later */
    if(PINF & (1 << 0)){
      mode = 't';
    } 
    else mode = 'r';
    /* if receiving, receive */
    if (mode == 'r'){
      uart_putchar(mode);   
      uint8_t radioStatus = 0;
      getRadioStatus(&radioStatus);
      if(getRX_DR()){  // if RX data received

        clearRX_DR();
        getRadioRXPayload(load, PACKET_SIZE); //read payload
        CE_LOW;
        for(int i = 0; i<PACKET_SIZE; i++){
          uart_putstring(load);
        }
      }
      CE_HIGH; 
      _delay_ms(5);
    }
    /* else transmit */
    else if(mode == 't'){
      transmit(data);
    }
  }
  free(load);
}
