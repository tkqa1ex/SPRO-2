#ifndef OPTOCOUPLER_H
#define OPTOCOUPLER_H

#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

/* =========================
   Configuration Constants
   ========================= */

#define DBS_TIME ((uint32_t)80000)

#define NUMBER_OF_HOLES 10
#define ANGLE (360.0 / (NUMBER_OF_HOLES * 2))

#define PI 3.14159
#define WHEEL_RADIUS 0.031   // 31 mm radius

/* Requires TIME_TICK and SCALAR from constants.h */
#define TIME_TICK_WITH_SCALAR (TIME_TICK * SCALAR)

/* =========================
   Global Variables
   ========================= */

extern volatile float DISTANCE_CHANGING_EDGE;

extern volatile int16_t cnt_edge_changed;

extern volatile uint32_t cnt_timer_overflow;
extern volatile uint32_t last_trigger;
extern volatile uint32_t time_on;

/* =========================
   Function Prototypes
   ========================= */

/**
 * @brief Initialize optocoupler input and interrupts.
 */
void optocoupler_init(void);

#endif /* OPTOCOUPLER_H */