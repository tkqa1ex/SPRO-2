#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "i2cmaster.h"
#include "usart.h"
#include "ina219.h"
#include "tmp117.h"

//shunt configuration
#define CONTROL_BOARD_SHUNT      0.025    // 0.025 Ohm shunt on control board
#define GENERATOR_BOARD_SHUNT    1.0      // 1.0 Ohm shunt on generator board

//INA219 configuration magic numbers
//both configurations enable 32V Bus mode and 128x internal hardware averaging
#define CONFIG_CTRL_PGA1_40MV    0x39FF   // PGA/1: ±40mV range (max accuracy for motor loop)
#define CONFIG_GEN_PGA8_320MV    0x3FFF   // PGA/8: ±320mV range 



//pin definitions (j1 connector)
#define LOAD_DDR   DDRB
#define LOAD_PORT  PORTB

#define PIN_D9     PB1   // J1 Pin 2
#define PIN_D10    PB2   // J1 Pin 4
#define PIN_D11    PB3   // J1 Pin 6
#define PIN_D12    PB4   // J1 Pin 8
#define PIN_D13    PB5   // J1 Pin 10

// manual load selection
#define ACTIVE_TEST_LOAD  1

//load selection engine
void apply_test_load(uint8_t test_selection) 
{
    //clear all load lines to 0V first (safety reset)
    LOAD_PORT &= ~(_BV(PIN_D9) | _BV(PIN_D10) | _BV(PIN_D11) | _BV(PIN_D12) | _BV(PIN_D13));
    switch(test_selection) 
    {
        case 0:
            break;
            
        case 1:
            LOAD_PORT |= _BV(PIN_D9);
            break; 
            
        case 2:
            LOAD_PORT |= _BV(PIN_D10);
            break;
            
        case 3:
            LOAD_PORT |= _BV(PIN_D11);
            break;
            
        case 4:
            LOAD_PORT |= _BV(PIN_D12);
            break;
            
        case 5:
            LOAD_PORT |= _BV(PIN_D13);
            break;

        case 6:
            LOAD_PORT |= _BV(PIN_D9) | _BV(PIN_D12);
            break;
            
        default:
            break;
    }
}

// //global variables from other files
// extern volatile uint32_t cnt_timer_overflow;
// //Timer1 overflow interrupt (for optocoupler timing)
// ISR(TIMER1_OVF_vect)
// {
//     cnt_timer_overflow++; // increment overflow tracking counter when Timer1 hits 0xFFFF
// }

//main program
int main(void)
{
    float controlVoltage, controlCurrent, controlTemperature;
    float generatorVoltage, generatorCurrent, generatorTemperature;

    //initialization routines
    uart_init();
    io_redirect();
    i2c_init();

    //set J1 load pins as outputs and apply the selected test load
    LOAD_DDR |= _BV(PIN_D9) | _BV(PIN_D10) | _BV(PIN_D11) | _BV(PIN_D12) | _BV(PIN_D13);
    apply_test_load(ACTIVE_TEST_LOAD);

    _delay_ms(100); //time for power to stabilize

    //initialize chips with their specific PGA configuration modes
    INA219_init(INA219_CONTROL_BOARD_ADDR, CONFIG_CTRL_PGA1_40MV);
    INA219_init(INA219_GENERATOR_BOARD_ADDR, CONFIG_GEN_PGA8_320MV);
    
    TMP117_init(TMP117_CONTROL_BOARD_ADDR);
    TMP117_init(TMP117_GENERATOR_BOARD_ADDR);

    //sei();
    _delay_ms(200);

    while (1)
    {
        //1. read Voltages
        controlVoltage   = INA219_getBusVoltage(INA219_CONTROL_BOARD_ADDR);
        generatorVoltage = INA219_getBusVoltage(INA219_GENERATOR_BOARD_ADDR);
        _delay_ms(5);

        //2. calculate Control Board Current (PGA /1)
        //multiply by 0.01 to convert raw bits directly into millivolts
        float ctrlShuntV = INA219_getShuntVoltage(INA219_CONTROL_BOARD_ADDR) * 0.01;
        controlCurrent   = (ctrlShuntV / CONTROL_BOARD_SHUNT);
        _delay_ms(5);

        //3. calculate Generator Board Current (PGA/8)
        //multiply by 0.01 and scale by 4.0 to correct bit alignment for PGA/8 mode
        float genShuntV  = INA219_getShuntVoltage(INA219_GENERATOR_BOARD_ADDR) * 0.01;
        generatorCurrent = (genShuntV / GENERATOR_BOARD_SHUNT);
        _delay_ms(5);

        //4. read temperatures
        controlTemperature   = TMP117_readTemperature(TMP117_CONTROL_BOARD_ADDR);
        _delay_ms(5);
        generatorTemperature = TMP117_readTemperature(TMP117_GENERATOR_BOARD_ADDR);

        //output clean telemetry data to your serial monitor
        printf("TEST_LOAD_MODE: %d | CTRL: %.2fV %.2fmA %.2fC | GEN: %.2fV %.2fmA %.2fC\n", 
               ACTIVE_TEST_LOAD,
               controlVoltage, controlCurrent, controlTemperature,
               generatorVoltage, generatorCurrent, generatorTemperature);

        _delay_ms(1000);
    }
}