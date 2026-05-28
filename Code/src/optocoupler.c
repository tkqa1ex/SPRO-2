#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "optocoupler.h"

// configuration macros
#define DBS_TIME (uint32_t)80000 // software debounce timeout threshold using timer ticks
#define NUMBER_OF_HOLES 10
#define ANGLE 360.0 / (NUMBER_OF_HOLES * 2)
#define PI 3.14159
#define WHEEL_RADIUS 0.031 // 31 mm physical wheel radius (stored in meters)

//volatile variables used directly in ISR and background math loops
volatile float DISTANCE_CHANGING_EDGE = (2 * PI * WHEEL_RADIUS) * ANGLE / 360.0;
volatile int16_t cnt_edge_changed;
volatile uint32_t cnt_timer_overflow;
volatile uint32_t last_trigger;
volatile uint32_t time_on;

// pin change interrupt 0 vector (triggers when PB0 toggles)
ISR(PCINT0_vect)
{
    static int last_edge = 0; // keeps track of the previous state to detect a real transition
    // combine current Timer1 overflow count with the raw 16-bit Timer1 counter (TCNT1)
    uint32_t actual_time = (cnt_timer_overflow << 16) | TCNT1;
    // read current state of input pin PB0 (1 = high, 0 = low)
    int curent_edge = ((PINB & (1 << PB0)) > 0);
    // software debouncing check: only process if the pin state changed and enough time passed
    if (last_edge != curent_edge && actual_time - last_trigger > DBS_TIME) {  
        last_edge = curent_edge; 
        if (last_edge) { // on a rising edge, the solid slot block has passed completely
            time_on = actual_time - last_trigger; // calculate how long the slot was covered
            ++cnt_edge_changed;                   // increment the speed pulse counter
        }
        last_trigger = actual_time; // save the current time timestamp for the next check
    }
}

// initialization
void optocoupler_init() {
    //set PB0 pin explicitly as an input line
    DDRB &= ~(1 << PB0); 

    //enable pin change interrupts globally for port B
    PCICR |= (1 << PCIE0);     // enable pin change interrupt bank 0 (PCINT0 to PCINT7)
    PCMSK0 |= (1 << PCINT0);   // choose pin PB0 specifically to trip the interrupt vector

    //initialize Timer1 with a 1024 prescaler divider
    //at 16MHz clock, Timer1 will overflow roughly once every 4.19 seconds
    TCCR1B |= (1 << CS10) | (1 << CS12); 
}