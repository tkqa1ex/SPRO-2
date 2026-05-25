#ifndef OPTOCOUPLER_H
#define OPTOCOUPLER_H

// -----------------------------------------------------------------------------
// Macros & constants
// -----------------------------------------------------------------------------
#define DB_TICKS 64
#define SCALAR 1024
#define TIME_TICK_WITH_SCALAR (TIME_TICK * SCALAR)

extern bool SHOW_SPEED;
extern const float DISTANCE;

// -----------------------------------------------------------------------------
// Global variables (extern since they are defined in .c file)
// -----------------------------------------------------------------------------
extern volatile int cnt;
extern volatile int changed;
extern volatile int overflow;
extern volatile int time_logs_len;

extern volatile uint64_t timer_overflow;
extern volatile uint64_t tm;
extern volatile uint64_t last_tm;
extern volatile uint64_t time_button_pressed;
extern volatile uint64_t time_logs[20];

extern volatile bool change_pwm;

extern volatile float speed;
extern volatile float new_pwm;
extern volatile float division_T_D;

// -----------------------------------------------------------------------------
// Function prototypes
// -----------------------------------------------------------------------------
void optocoupler_init(void);
void record_time(void);

// ISR declarations (implemented in .c file)
ISR(PCINT0_vect);
ISR(TIMER1_OVF_vect);

#endif // OPTOCOUPLER_H