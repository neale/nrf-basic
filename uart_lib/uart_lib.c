#include "uart_lib.h"
static void init_uart(void){

  UBRR1 = 8; //38.4k
  UCSR1A = (1 << U2X1);
  UCSR1B = ((1 << RXEN1) | (1 << TXEN1));
  UCSR1C = ((1 << UCSZ11) | (1 << UCSZ10));
}
static void uart_putchar(char data){

  while(!(UCSR1A & (1 << UDRE1)));
  UDR1 = data;
}
static void uart_putstring(char * data){
  
int i = 0;
  
  while(data[i] != '\0'){
   while(!(UCSR1A & (1 << UDRE1)));
   UDR1 = data[i];
   i++;
  }
}
static void uart_flush(void){

  unsigned char temp;
  while(UCSR1B & (1 << RXC1)); //flush buffer
  temp = UDR1; //turn receive back on
}
static void hex_to_ascii(uint8_t hex){

    hex &= 0x0F;

    if(hex < 10){
      hex = hex + '0';
    }
    else{
      hex = hex + 'A' - 10;
    }
    uart_putchar((char)hex);
}
uint8_t main(void){
  
  init_uart();
  char data[8] = {'a', 'a', 'a', 'a', 'a', 'a', 'a', '\0'};
  uint8_t hex = 0x0F;
  uint8_t hex_string[3] = {0xFF, 0xAB, 0xBC};
  char ascii;
  hex_to_ascii(hex);
  uart_putchar(ascii);
  uart_putstring(data);
  uart_putchar('a');
  uart_flush();
   
  return 0;
}
