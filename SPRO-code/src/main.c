#include <stdio.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include "usart.h"
#include "i2cmaster.h"
#include "constants.h"
#include "INA219.h"
#include "TMP117.h"
#include "optocoupler.h"

volatile uint8_t timer;
volatile uint8_t take_data;

float bus_voltage_control,bus_voltage_generator,current_control,current_generator;
float ambiental_temperature_control,ambiental_temperature_generator,rotations_ms;

ISR(TIMER2_OVF_vect) {
    timer++;
    if (timer == SAMPLING_RATE) {
        timer = 0;
        take_data = true;
    }
}

ISR(TIMER1_OVF_vect) {
    cnt_timer_overflow++;
}

void take_measurement();
void init_timer2();
void init_outsiders();
void OPEN(pin LOAD);
void system_init(pin LOAD);
void get_TMP117_CONTROL();
void get_INA219_GENERATOR();
void get_INA219_CONTROL();
void get_rotating_speed();
void sent_data_to_matlab();

int main() {
    system_init(D9);
    while (1) {
        if (take_data) {
            take_measurement();
            send_data_to_matlab();
            take_data = false;
        }
    }
}

void sent_data_to_matlab() {
    // voltage control, current control, control temp
    // voltage generator, current generator, gen temp
    // rpms
    printf("%f %f %f\n",bus_voltage_control,current_control,ambiental_temperature_control);
    printf("%f %f %f\n",bus_voltage_generator,current_generator,ambiental_temperature_generator);
    printf("%f \n",rotations_ms);
}

void init_outsiders() {
    INA219_INIT_CONTROL();
    INA219_INIT_GENERATOR();
    TMP117_INIT(TMP117_CONTROL);
    TMP117_INIT(TMP117_GENERATOR);  
    optocoupler_init();

    // give enough time for components to wake up and transients to magically disappear
    // just to make sure they have the time to update, shuold be much less but we arent rly in a worry :)
    _delay_ms(50); 
}

void get_INA219_GENERATOR() {
    bus_voltage_generator = GET_BUS_VOLTAGE(INA219_GENERATOR);
    current_generator = GET_CURRENT_GENERATOR();
}

void get_TMP117_GENERATOR() {
    ambiental_temperature_control = TMP117_readTemperature(TMP117_GENERATOR);
}

void get_INA219_CONTROL() {
    bus_voltage_control = GET_BUS_VOLTAGE(INA219_CONTROL); // seems to be overshoot by 0.2V
    current_control = GET_CURRENT_CONTROL();
}

void get_rotating_speed() {
    rotations_ms = SAMPLING_RATE / (1.0 * cnt_edge_changed);
}

void take_measurement() {
    get_INA219_CONTROL();
    get_INA219_GENERATOR();
    get_TMP117_CONTROL();
    get_TMP117_GENERATOR();
    get_rotating_speed();
}

void init_timer2() {
    // SCALAR OF 64, lasts 1ms
    TIMSK2 |= (1 << TOIE2);
    TCCR2B |= (1 << CS22); 
}

void system_init(pin LOAD) {
    sei(); // enable interrupts

    // debug + sending to matlab
    uart_init();
    io_redirect();

    // enable i2c communication
    i2c_init(); 

    // enable the load
    OPEN(LOAD);

    // init sensors
    init_outsiders();    

    // timer go brrr
    init_timer2();
}

void OPEN(pin LOAD) {
    // open the D9-D13 pin (PB1-5) resistor
    DDRB |= (1 << (LOAD + 1));
    PORTB |= (1 << (LOAD + 1));
}



