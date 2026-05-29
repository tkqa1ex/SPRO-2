#ifndef CONSTANTS_H
#define CONSTANTS_H

// sensor addresses
typedef enum { 
    INA219_CONTROL = (uint8_t) 0b1000000,
    INA219_GENERATOR = (uint8_t) 0b1000001,
    TMP117_GENERATOR = (uint8_t) 0b1001001,
    TMP117_CONTROL = (uint8_t) 0b1001000
}SLAVE_ADDRESS;


#define ARDUINO_FREQUENCY (16000000.0f)
#define TIME_TICK (1.0 / 16000000.0)
#define MAXIMUM_u32 (uint64_t)4294967295
#define MAXIMUM_u16 ((uint32_t)65535)
#define MAXIMUM_u8 ((uint16_t)255)
#define SAMPLING_RATE (uint8_t) 275 // time in ms between measurements

// new pin type to select the load easily by the name of the actual pin
typedef enum {
    D9,
    D10,
    D11,
    D12,
    D13
}pin;

#endif 