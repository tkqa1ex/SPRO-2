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


volatile uint8_t timer;
volatile uint8_t take_data;

float bus_voltage_control,bus_voltage_generator,current_control,current_generator;

void get_INA219_CONTROL() {
    bus_voltage_control = GET_BUS_VOLTAGE(INA219_CONTROL) - 0.1; // seems to be overshoot by 0.2V
    current_control = GET_CURRENT_CONTROL();
}

void get_INA219_GENERATOR() {
    bus_voltage_generator = GET_BUS_VOLTAGE(INA219_GENERATOR);
    current_generator = GET_CURRENT_GENERATOR();
}
// D9 - 0.5
// D10 - 
void take_measurement() {
    cli();
    get_INA219_CONTROL();
    get_INA219_GENERATOR();
    get_TMP117_CONTROL();
    get_TMP117_GENERATOR();
    printf("For CONTROL: Bus voltage is %.5fV Current is %.5fmA\n",bus_voltage_control,current_control);
    printf("For GENERATOR: Bus voltage is %.5fV Current is %.5fmA\n \n \n",bus_voltage_generator,current_generator);
    _delay_ms(500);
    sei();
    // get_TMP117_CONTROL();
    // get_TMP117_GENERATOR();
}


ISR(TIMER2_OVF_vect) {
    timer++;
    if (timer == SAMPLING_RATE) {
        timer = 0;
        take_data = true;
    }
}



void init_timer() {
    // SCALAR OF 64, lasts 1ms
    TIMSK2 |= (1 << TOIE2);
    TCCR2B |= (1 << CS22); 
}

void init_outsiders() {
    INA219_INIT_CONTROL();
    INA219_INIT_GENERATOR();
    // TMP117_CONTROL_INIT();
    // TMP117_GENERATOR_INIT();

    // might have to add sth for FAN and HEATING PAD too... :(
    _delay_ms(100); // just to make sure they have the time to update, shuold be much less but we arent rly in a worry :)
}

void OPEN(pin LOAD) {
    // open the D9-D13 pin (PB1-5) resistor
    DDRB |= (1 << (LOAD + 1));
    PORTB |= (1 << (LOAD + 1));
}

void system_init(pin LOAD) {
    sei(); // enable interrupts
    _delay_ms(500); // give enough time for components to wake up

    // debug
    uart_init();
    io_redirect();

    // enable i2c communication
    i2c_init(); 

    // enable the load
    OPEN(LOAD);
    _delay_ms(5000);

    // init sensors
    init_outsiders();    

    // timer go brrr
    init_timer();
}

// current - control -> 
// voltage - control ->
// current - generator ->
// voltage - generator ->

int main() {
    system_init(D9);
    while (1) {
        if (true) {
            take_measurement();
            
            //send_data_to_matlab();
            take_data = false;
        }
    }
}
// gonna give like 500ms of pause so components complete their POR phase
// collect data every 200ms => 5 samples/s
// could go lower but temp updates only once 125ms
// INA goes for 12bit even less than a ms


/*
takes aprox 1.5ms after turning on to be usable
UNFORTUNATELY CANT USE ANYTH THATS FOR ALERT (to create interrupts) BECAUSE OURS IS LEFT FLOATING :(

can write in the eeprom to configure it
BY DEFAULT IT IS LOCKED: Set Bit 15 of the EEPROM Unlock Register to 1 to Unlock
                        Write
                        WAIT 7ms
                        Read EEPROM_Busy from EEPROM Unlock Register, if busy wait 7ms again

The 8-bit pointer register of the device is used to address a given data register
    - deci e efectiv un pointer, aici vei da store la adresa registrului dorit
    - but i mean asa e in general, aici doar iti si spune cum e stocat, aceasi marie cu alta palarie

pag 22: shows reg and their function
LSB - 7.8125mC

Temp reg -> 0x00
    LSB - 7.8125mC
    Following power-up, before first conversion ends it has -256C. Way to check it works!!!!

Config reg -> 0x01
    bit 13 - DATA_READY FLAG
    bit 12 - EEPROM BUSY
    bit 10-11 -> conversion mode (again continuous cause here it rly doesnt matter as much)
    bit 7-9 -> time for conversion (dont need to be 100% accurate so one reading is enough) -> 125ms is fine
    bit 5-6 -> AVG = 00 (no average, one sample is enough)
    bit 2-> DATA_READY

High Limit Reg -> 0x02
    LSB - 7.8125mC

Low Limit Reg -> 0x03
    same LSB

Temp Offset Reg -> 0x07
    dont need to touch it, its used to give more accurate readings but its very primitive, just offset the result by a value
    we dont need that much accuracy so should be fine, but if we notice it usually gives more (or less, maybe could make it less caus the motor heats harder) we can use this

EEPROM Unlock Register -> 0x04
    bit 15: EEPROM UNLOCK bit
    bit 14: EEPROM BUSY

3 EEPROM registers in case we need
+ also in EEPROM ->
DEVICE ID REG (to ensure again proper communication) it is in the eeprom -> 0x0F

*/