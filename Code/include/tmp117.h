#ifndef TMP117_H
#define TMP117_H

#include <stdint.h>

// Ensure these match your raw 7-bit hex addresses (e.g., 0x48, 0x49, etc.)
#define TMP117_CONTROL_BOARD_ADDR     0x48  
#define TMP117_GENERATOR_BOARD_ADDR   0x49  

void TMP117_init(uint8_t addr);
float TMP117_readTemperature(uint8_t addr);

#endif