#ifndef TMP117_H
#define TMP117_H

#include <stdint.h>

void TMP117_INIT(uint8_t addr);
float TMP117_readTemperature(uint8_t addr);

#endif