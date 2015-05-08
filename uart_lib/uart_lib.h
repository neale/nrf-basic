#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#define F_CPU 16000000


static void init_uart(void);
static void uart_putchar(char data);
static void uart_putstring(char *data);

