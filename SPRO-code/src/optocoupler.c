#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "constants.h"

#define  TIME_TICK_WITH_SCALAR (TIME_TICK * SCALAR)
#define ANGLE 18
#define PI 3.14159
#define WHEEL_RADIUS 0.031 // 31 mm radius

volatile float DISTANCE_CHANGING_EDGE = (2 * PI * WHEEL_RADIUS) * ANGLE / 360.0;
const static uint32_t SCALAR = 1; // scalar of clock


bool SHOW_SPEED;
const static float DISTANCE = 100;
volatile int cnt,changed,overflow;
volatile uint32_t new_time,old_time;
volatile float speed;


ISR(TIMER1_CAPT_vect) {
    static int was = -1;
    new_time = ICR1;
    volatile uint16_t now = PINB & (1 << PINB0);
    float tm = 0;
    if (new_time >= old_time) {
        tm = new_time - old_time + overflow *  MAXIMUM_u16;
    } else {
        tm = MAXIMUM_u16 - old_time + new_time + overflow * MAXIMUM_u16;
    }
    tm = TIME_TICK_WITH_SCALAR * tm;
    if (tm > 0.05) { // debounce of 50ms
        if (!was && now) { // edge is increasing
             ++cnt; 
            TCCR1B &= ~(1 << ICES1);
            overflow = 0;
            old_time = new_time;
        } else if (was && !now) {
            speed = DISTANCE / (float)tm;
            TCCR1B |= (1 << ICES1);
            changed = 1;
        }
    }
    was = now;
}

void optocoupler_init() {
    DDRB &= ~(1 << PB0);
    PORTB |= (1 << PB0);
    TCCR1B |= (1 << ICES1);
    // enable timer for increasing edge + clockage
    TCCR1B |= (1 << ICES1) | (1 << CS10);
    TIMSK1 |= (1 << ICIE1) | (1 << TOIE1);
    // enable types of interruptors -> change of flag + overflow of counter
    sei();
}
