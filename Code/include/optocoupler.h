#ifndef optocoupler_h
#define optocoupler_h

#include <stdint.h>

//global variables
extern volatile float DISTANCE_CHANGING_EDGE;
extern volatile int16_t cnt_edge_changed;
extern volatile uint32_t cnt_timer_overflow;
extern volatile uint32_t last_trigger;
extern volatile uint32_t time_on;

//function prototypes
// initialize optocoupler input pin and pin-change interrupts
void optocoupler_init(void);

#endif 