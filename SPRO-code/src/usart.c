#include <avr/io.h>
#include <stdio.h>
#include "usart.h"

#ifndef F_CPU
#define F_CPU 16000000UL
#endif

#ifndef BAUD
#define BAUD 9600
#endif
#include <util/setbaud.h>
#include <avr/interrupt.h>


#define BUFFER_SIZE 64

static volatile char rx_buffer[BUFFER_SIZE];
static volatile uint8_t rx_head = 0, rx_tail = 0;
static volatile uint8_t rx_count = 0;

FILE f_uart = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);


void uart_init(void) {
	
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	
	#if USE_2X
	UCSR0A |= _BV(U2X0);
	#else
	UCSR0A &= ~(_BV(U2X0));
	#endif

	UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); /* 8-bit data */
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);   /* Enable RX and TX and RX interrupt*/
	
}

void io_redirect(void ){
		
		
		stdout = &f_uart;	//redirect standard output to uart
		stdin  = &f_uart;  //redirect standard input to uart
}
	

int uart_putchar(char c, FILE *stream) {
	if (c == '\n') {
		uart_putchar('\r', stream);
	}
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}

int uart_getchar(FILE *stream) {
	loop_until_bit_is_set(UCSR0A, RXC0);
	if (UCSR0A & _BV(FE0))
		return _FDEV_EOF;
	if (UCSR0A & _BV(DOR0))
		return _FDEV_ERR;
	// now USART has data available in buffer
	
	return UDR0;
}


// Initialize UART with interrupts


ISR(USART_RX_vect) {
    char data = UDR0;
    
    if (rx_count < BUFFER_SIZE) {
        rx_buffer[rx_head] = data;
        rx_head = (rx_head + 1) % BUFFER_SIZE;
        rx_count++;
    }
    // Buffer overflow - data lost
}

// Get character from buffer (non-blocking)
int uart_getchar_nonblocking(char *c) {
    if (rx_count == 0) {
        return 0; // No data available
    }
    
    *c = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % BUFFER_SIZE;
    rx_count--;
    
    return 1;
}

// Send character (blocking)
void uart_putchar2(char c) {
    UDR0 = c;
}