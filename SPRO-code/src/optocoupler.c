#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "constants.h"

#define DBS_TIME (uint32_t)80000
#define  TIME_TICK_WITH_SCALAR (TIME_TICK * SCALAR)
#define NUMBER_OF_HOLES 10
#define ANGLE 360.0 / (NUMBER_OF_HOLES * 2)
#define PI 3.14159
#define WHEEL_RADIUS 0.031 // 31 mm radius

volatile float DISTANCE_CHANGING_EDGE = (2 * PI * WHEEL_RADIUS) * ANGLE / 360.0;


volatile int16_t cnt_edge_changed;
volatile uint32_t cnt_timer_overflow,last_trigger,time_on;


// PCIN interrupt for changing edge
ISR(PCINT0_vect)
{
    // local vars dont need volatile
    static int last_edge = 0;

    uint32_t actual_time = (cnt_timer_overflow << 16) | TCNT1;
    int curent_edge = ((PINB & (1 << PB0)) > 0);
    // check if rising edge(1) or decreasing(0)
    if (last_edge != curent_edge && actual_time - last_trigger > DBS_TIME){  
        last_edge = curent_edge; 
        if (last_edge) { // on rising edge the solid part stops
            time_on = actual_time - last_trigger;
            ++cnt_edge_changed;
        }
        last_trigger = actual_time;
    }
}
void optocoupler_init() {
    DDRB &= ~(1 << PB0); 

    // enable pin to catch changing edges
    PCICR |= (1 << PCIE0); // enable for PCINT0
    PCMSK0 |= (1 << PCINT0); // select PCINT0 (PB0) -> interrupt for changing edge 

    // init timer1 with 1024 prescalar => rouhgly once every 4.2secs
    // if needed more time just modify the top
    TCCR1B |= (1 << CS10) | (1 << CS12); // 1024 prescalar
}
