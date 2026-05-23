#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "usart.h"

#define DBS_TIME (uint32_t)80000

float PWM_FAN = 0.5;
float PWM_HEATING_PAD = 0.25;
volatile uint32_t last_fan_button_press,last_heating_pad_button_press;
volatile uint16_t tovf_cnt,fan_adc,heating_pad_adc;
volatile uint8_t last_fan_edge = 1,last_heating_pad_edge = 1,adc_conversion_complete;

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
    uart_init();
    io_redirect();
    printf("Started\n");
    arduino_init();
    while (1) {
        read_new_ADC();
        adjust_PWM();
        _delay_ms(300);
    }
}


ISR(PCINT0_vect) {
    uint32_t actual_time = ((uint32_t)tovf_cnt << 16) + TCNT1;
    uint8_t current_edge = 0;
    if (PINB & (1 << PB1)) {
        current_edge = 1;
    }
    // on a falling edge
    if (last_fan_edge != current_edge) {
        if (last_fan_edge == 1 && actual_time - last_fan_button_press >= DBS_TIME) {
            last_fan_button_press = actual_time;
            if (OCR0B == 0) {
                OCR0B = PWM_HEATING_PAD * 0xFF;
            } else {
                OCR0B = 0;
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
    if (actual_time - last_heating_pad_button_press >= DBS_TIME && last_heating_pad_edge != current_edge) {
        if (last_heating_pad_edge == 1) {
            last_heating_pad_button_press = actual_time;
            if (OCR2B == 0) {
                OCR2B = PWM_FAN * 0xFF;
            } else {
                OCR2B = 0;
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
    OCR2B = 0xFF * pwm;
}

void set_fan_PWM(float pwm) {
    PWM_FAN = pwm;
    OCR0B = 0xFF * pwm;
}

void arduino_init() {
    ADMUX |= (1 << REFS0);
    ADCSRA |= (1 << ADEN);
    ADCSRA |= (1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0);
    // ADCSRA reg to enable

    // ADC has a resolution of 10 bits with 5V as VREF => we can use a 1k pot to change the pwm, nice :)
    // prescalar of 8, we get 32ms
    // TCNT
    TCCR1B |= (1 << CS11);
    //TOVF niterrupt
    TIMSK1 |= (1 << TOIE1);

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


    // gonna use PCINT22 (PD6-D6),PCINT1(PB1-D9) 
    // 1 -> FAN
    // 22 -> HEATING PAD
    DDRB &= ~(1 << PB1);
    DDRD &= ~(1 << PD6);

    PORTB |= (1 << PB1);
    PORTD |= (1 << PD6);
    // enable the 2 and 0 group
    PCICR |= (1 << PCIE2) | (1 << PCIE0);
    PCMSK2 |= (1 << PCINT22);
    PCMSK0 |= (1 << PCINT1);
    sei();
}

// can use the pins to the generator to track the i2c bus
// the current has 16 bit contents



 