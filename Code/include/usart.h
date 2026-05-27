#ifndef USART_H
#define USART_H

#include <stdio.h>

void uart_init(void);
void io_redirect(void);
int uart_putchar(char c, FILE *stream);
int uart_getchar(FILE *stream);

#endif