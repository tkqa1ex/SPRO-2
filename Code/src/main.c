#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#include "i2cmaster.h"
#include "usart.h"
#include "ina219.h"
#include "tmp117.h"

// =====================================================
// SHUNT VALUES
// =====================================================
#define CONTROL_BOARD_SHUNT      0.025
#define GENERATOR_BOARD_SHUNT    0.5

// =====================================================
// MANUAL LOAD TEST SELECTION
// =====================================================
#define ACTIVE_TEST_LOAD  1

// =====================================================
// HARDWARE PIN DEFINITIONS (J1 CONNECTOR)
// =====================================================
#define LOAD_DDR   DDRB
#define LOAD_PORT  PORTB

#define PIN_D9     PB1   // J1 Pin 2
#define PIN_D10    PB2   // J1 Pin 4
#define PIN_D11    PB3   // J1 Pin 6
#define PIN_D12    PB4   // J1 Pin 8
#define PIN_D13    PB5   // J1 Pin 10

// =====================================================
// LOAD SELECTION ENGINE
// =====================================================
void apply_test_load(uint8_t test_selection) 
{
    // Clear all load lines to 0V first (Safety reset)
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

// =====================================================
// MAIN PROGRAM
// =====================================================
int main(void)
{
    float controlVoltage;
    float controlCurrent;
    float controlTemperature;

    float generatorVoltage;
    float generatorCurrent;
    float generatorTemperature;

    uart_init();
    io_redirect();
    i2c_init();

    // Configure the 5 load control lines as outputs
    LOAD_DDR |= _BV(PIN_D9) | _BV(PIN_D10) | _BV(PIN_D11) | _BV(PIN_D12) | _BV(PIN_D13);

    apply_test_load(ACTIVE_TEST_LOAD);

    INA219_init(INA219_CONTROL_BOARD_ADDR);
    INA219_init(INA219_GENERATOR_BOARD_ADDR);
    TMP117_init(TMP117_CONTROL_BOARD_ADDR);
    TMP117_init(TMP117_GENERATOR_BOARD_ADDR);

    _delay_ms(500);

    while (1)
    {
        // Sample INA219 Control Metrics (Compensated for PGA /4 scaling factor shift)
        controlVoltage     = INA219_getBusVoltage(INA219_CONTROL_BOARD_ADDR);
        controlCurrent     = (INA219_getShuntVoltage(INA219_CONTROL_BOARD_ADDR) / CONTROL_BOARD_SHUNT) * 2.0;
        _delay_ms(5);

        // Sample INA219 Generator Metrics (Compensated for PGA /4 scaling factor shift)
        generatorVoltage   = INA219_getBusVoltage(INA219_GENERATOR_BOARD_ADDR);
        generatorCurrent   = (INA219_getShuntVoltage(INA219_GENERATOR_BOARD_ADDR) / GENERATOR_BOARD_SHUNT);
        _delay_ms(5);

        // Sample Temperatures
        controlTemperature   = TMP117_readTemperature(TMP117_CONTROL_BOARD_ADDR);
        generatorTemperature = TMP117_readTemperature(TMP117_GENERATOR_BOARD_ADDR);
        _delay_ms(5);

        printf("TEST_LOAD_MODE: %d | CTRL: %.2fV %.2fmA %.2fC | GEN: %.2fV %.2fmA %.2fC\n", 
               ACTIVE_TEST_LOAD,
               controlVoltage, controlCurrent, controlTemperature,
               generatorVoltage, generatorCurrent, generatorTemperature);

        _delay_ms(1000);
    }
}