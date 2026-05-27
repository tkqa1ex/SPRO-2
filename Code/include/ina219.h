#ifndef INA219_H
#define INA219_H

#include <stdint.h>

// Ensure these match your raw 7-bit hex addresses (e.g., 0x40, 0x41, 0x44, etc.)
#define INA219_CONTROL_BOARD_ADDR     0x40  
#define INA219_GENERATOR_BOARD_ADDR   0x41  

void INA219_init(uint8_t addr);
float INA219_getBusVoltage(uint8_t addr);
float INA219_getShuntVoltage(uint8_t addr);

#endif