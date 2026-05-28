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

// flag which makes sure the sampling rate is mantained 
volatile uint8_t take_data;

// for the data from the sensors
float bus_voltage_control,bus_voltage_generator,current_control,current_generator,
ambiental_temperature_control,ambiental_temperature_generator,rotations_ms;



// start timer with precalar of 64 to get 1ms per counter overflow
void init_timer2();

// configure all the sensors through I2C
void init_outsiders();
void OPEN(pin LOAD);
void system_init(pin LOAD);

// used to get the needed data
void take_measurement();
void get_TMP117_CONTROL();
void get_TMP117_GENERATOR();
void get_INA219_GENERATOR();
void get_INA219_CONTROL();
void get_rotating_speed();

// send data to the PC which reads it with matlab
void send_data_to_matlab();

ISR(TIMER2_OVF_vect) {
    // every count is 1ms
    timer++;
    // to ensure the sampling rate
    if (timer == SAMPLING_RATE) {
        timer = 0;
        take_data = true;
    }
}

ISR(TIMER1_OVF_vect) {
    // every count is around 4.2s
    cnt_timer_overflow++;
}

int main() {
    // initialise the system whilst also opening pin D9
    system_init(D9);
    while (1) {
        // whenever the sampling time is reached take_data is set true by
        // a timer2 interrupt
        if (take_data) {
            // get all the measurements from the sensors
            take_measurement();

            // send everything through usart to 
            send_data_to_matlab();
            take_data = false;
        }
    }
}

void send_data_to_matlab() {
    // Sending to the PC data as follows
    // voltage control, current control, control temp
    // voltage generator, current generator, gen temp
    // rpms
    printf("Control: bus voltage: %f V current: %f mA temperature: %fC\n",bus_voltage_control,current_control,ambiental_temperature_control);
    printf("Generator: bus voltage: %f V current: %f mA temperature: %fC\n",bus_voltage_generator,current_generator,ambiental_temperature_generator);
    printf("Rotations: %f \n\n\n",rotations_ms);
    cnt_edge_changed = 0;
}

// init sensors
void init_outsiders() {
    INA219_INIT_CONTROL();
    INA219_INIT_GENERATOR();
    TMP117_INIT(TMP117_CONTROL);
    TMP117_INIT(TMP117_GENERATOR);  
    // if optocoupler needed
    //optocoupler_init();
    // was to debug and make sure the config has the right value
    // GET_INA219_REG(INA219_CONTROL,CONFIGURATION);
    // GET_INA219_REG(INA219_GENERATOR,CONFIGURATION);

    // give enough time for components to wake up and transients to magically disappear, to fade away like my hopes and dreams :)))
    // just to make sure they have the time to update, shuold be much less but we arent rly in a worry :)
    _delay_ms(5); 
}

// get readings from the INA219 sensor from the generator
void get_INA219_GENERATOR() {
    bus_voltage_generator = GET_BUS_VOLTAGE(INA219_GENERATOR);
    current_generator = GET_CURRENT_GENERATOR();
}

// get readings from the TMP117 sensor from the generator
void get_TMP117_GENERATOR() {
    ambiental_temperature_generator = TMP117_readTemperature(TMP117_GENERATOR);
}

// get readings from the TMP117 sensor from the control
void get_TMP117_CONTROL() {
    ambiental_temperature_control = TMP117_readTemperature(TMP117_CONTROL);
}

// get readings from the INA219 sensor from the control
void get_INA219_CONTROL() {
    bus_voltage_control = GET_BUS_VOLTAGE(INA219_CONTROL); // seems to be overshoot by 0.2V
    current_control = GET_CURRENT_CONTROL();
}

// in case optocoupler is needed for rpm
void get_rotating_speed() {
    rotations_ms = SAMPLING_RATE / (1.0f * cnt_edge_changed);
}

// get the data from the sensors to the mcu
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



