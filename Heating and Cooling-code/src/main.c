#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "usart.h"

#define DBS_TIME (uint32_t)10000

volatile float PWM_FAN = 0.5;
volatile float PWM_HEATING_PAD = 0.25;
volatile uint32_t last_fan_button_press,last_heating_pad_button_press;
volatile uint16_t tovf_cnt,fan_adc,heating_pad_adc;
volatile uint8_t last_fan_edge = 1,last_heating_pad_edge = 1,adc_conversion_complete;
volatile uint8_t change_pwm_fan = 1,change_pwm_heating_pad = 1;

// implement 2 pwms (hai T0 si T2 sa aiba aceeasi durata)
// check they work
// add PCINT to arduino and check with buttons
// beers?

void arduino_init(void);

// not used but nice to have in case u want to implement a PID controller
void set_heating_pad_PWM(float pwm);
void set_fan_PWM(float pwm);
void adjust_PWM();
void read_new_ADC();
uint16_t read_adc(int);

int main() {
    arduino_init();
    while (1) {
        // read the new voltages to figure out position of the pot
        // and change the pwm accordingly
        read_new_ADC();
        adjust_PWM();
        _delay_ms(300);
    }
}

// same structure for the PCINT2 ISR
ISR(PCINT0_vect) {
    // the real time at the time this interrupt was triggered
    uint32_t actual_time = ((uint32_t)tovf_cnt << 16) + TCNT1;

    // find what type of change has occured (1 means rising, 0 is falling)
    uint8_t current_edge = 0;
    if (PINB & (1 << PB1)) {
        current_edge = 1;
    }

    // only process the interrupt when the edge actually changes
    // noise can sometimes toggle the signal extremely quickly.
    //
    // for example
    // - last valid edge was HIGH
    // - noise briefly causes HIGH -> LOW -> HIGH within a few us
    // the temporary LOW transition is ignored by the debounce filter because it
    // occurs too soon after the previous HIGH edge. However, the second HIGH may
    // arrive later and still trigger a pin-change interrupt even though the
    // logical edge state never truly changed.
    if (last_fan_edge != current_edge) {
        // if it is on a rising edge and the time since the last press is more than the DBS_TIME 
        if (last_fan_edge == 1 && actual_time - last_fan_button_press >= DBS_TIME) {
            last_fan_button_press = actual_time;
            if (change_pwm_fan == 0) {
                change_pwm_fan = 1;
                OCR0B = PWM_FAN * 0xFF;
                TCCR0B |= (1 << CS01) | (1 << CS00); 
            } else {
                change_pwm_fan = 0;
                OCR0B = 0;
                TCCR0B = 0;
            }
        }
        last_fan_edge = current_edge;
    }
}

ISR(PCINT2_vect) {
    uint32_t actual_time = ((uint32_t)tovf_cnt << 16) + TCNT1;
    uint8_t current_edge = 0;
    if (PIND & (1 << PD6)) {
        current_edge = 1;
    }
    if (last_heating_pad_edge != current_edge) {
        if (last_heating_pad_edge == 1 && actual_time - last_heating_pad_button_press >= DBS_TIME) {
            last_heating_pad_button_press = actual_time;
            if (change_pwm_heating_pad == 0) {
                change_pwm_heating_pad = 1;
                TCCR2B |= (1 << CS22);
                OCR2B = PWM_HEATING_PAD * 0xFF;
            } else {
                change_pwm_heating_pad = 0;
                OCR2B = 0;
                TCCR2B = 0;
            }
        }
        last_heating_pad_edge = current_edge;
    }
}

ISR(TIMER1_OVF_vect) {
    ++tovf_cnt;
}

uint16_t read_adc(int channel) {
    ADMUX &= ~(0x0F);
    ADMUX |= channel;
    _delay_ms(10);
    ADCSRA |= (1 << ADSC);
    while (ADCSRA & (1 << ADSC));

    return ADC;
}
// gonna use ADC0-1 -> PC0-1 (A0-1)
void read_new_ADC() {
    fan_adc = read_adc(0);
    heating_pad_adc = read_adc(1);
}

void adjust_PWM() {
    // x - 100
    // ADC - 2^10
    set_fan_PWM(fan_adc / 1024.0);
    set_heating_pad_PWM(heating_pad_adc / 1024.0);
    //printf("FAN PWM: %.2f\n HEATING PAD PWM: %.2f \n\n",PWM_FAN,PWM_HEATING_PAD);
}

void set_heating_pad_PWM(float pwm) {
    PWM_HEATING_PAD = pwm;
    if (change_pwm_heating_pad) {
        OCR2B = 0xFF * pwm;
    }
}

void set_fan_PWM(float pwm) {
    PWM_FAN = pwm;
    if (change_pwm_fan) {
        OCR0B = 0xFF * pwm;
    }
}

void enable_ADC() {
    ADMUX |= (1 << REFS0);
    ADCSRA |= (1 << ADEN);
    ADCSRA |= (1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0);
    // ADC has a resolution of 10 bits with 5V as VREF => we can use a 1k pot to change the pwm, nice :)
}

void init_timer() {
    // init timer1 with prescalar of 8 to get
    // 32ms
    TCCR1B |= (1 << CS11);
    //TOVF niterrupt
    TIMSK1 |= (1 << TOIE1);
}

void set_up_pwms() {
    // use fast pwm + toggle on compare match
    // gonna use OC0B with clear on equal
    // setting for OC0B -> PD5, CHECKED, click of button also CHECKED
    DDRD |= (1 << PD5);
    OCR0B = 0xFF * PWM_FAN;
    TCCR0A |= (1 << COM0B1) | (1 << WGM01) | (1 << WGM00);
    // scalar of 64 so every overflow is 1ms 
    TCCR0B |= (1 << CS01) | (1 << CS00); 
    

    // setting for 0C2B -> PD3, CHECKED
    DDRD |= (1 << PD3);
    OCR2B = 0xFF * PWM_HEATING_PAD;
    TCCR2A |= (1 << COM2B1) | (1 << WGM21 | 1 << WGM20);
    TCCR2B |= (1 << CS22);
}

void set_up_edge_detection() {
    // gonna use PCINT22 (PD6-D6),PCINT1(PB1-D9) 
    // 1 -> FAN (PB1)
    // 22 -> HEATING PAD (PD6)
    DDRB &= ~(1 << PB1);
    DDRD &= ~(1 << PD6);

    PORTB |= (1 << PB1);
    PORTD |= (1 << PD6);
    // enable the 2 and 0 group
    PCICR |= (1 << PCIE2) | (1 << PCIE0);
    PCMSK2 |= (1 << PCINT22);
    PCMSK0 |= (1 << PCINT1);
}

void arduino_init() {
    // initialise the communication to the pc for debugging purposes mainly
    uart_init();
    io_redirect();

    enable_ADC();
    init_timer();
    set_up_pwms();
    set_up_edge_detection();
    // enable interrupts
    sei();
}

// can use the pins to the generator to track the i2c bus
// the current has 16 bit contents



 