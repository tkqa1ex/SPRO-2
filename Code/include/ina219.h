#ifndef INA219_H_
#define INA219_H_

#include <stdint.h>

// device slave addresses
#define INA219_CONTROL_BOARD_ADDR     0x40  // I2C address for the motor control board
#define INA219_GENERATOR_BOARD_ADDR   0x41  // I2C address for David's generator board

//function prototypes

//initialization 
//now takes a second parameter (config_value) to pass custom PGA modes over I2C
void INA219_init(uint8_t addr, uint16_t config_value);

//read main supply voltage
//returns a float value scaled directly into Volts
float INA219_getBusVoltage(uint8_t addr);

//read pure raw data from the shunt register
//returns unshifted raw integer from register 0x01 so we can scale it manually in main.c
float INA219_getShuntVoltage(uint8_t addr);

#endif