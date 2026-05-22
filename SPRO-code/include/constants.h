#ifndef CONSTANTS_H
#define CONSTANTS_H

// sensor addresses
typedef enum { // efectiv creezi un tip nou, aaa si valoarea variabilei e ordinea nice
    INA219_CONTROL = (uint8_t) 0b1000000,
    INA219_GENERATOR = (uint8_t) 0b1000001,
    TMP117_GENERATOR = (uint8_t) 0b1001001,
    TMP117_CONTROL = (uint8_t) 0b1001000
}SLAVE_ADDRESS;

#define SAMPLING_RATE (uint8_t) 275 // time in ms between measurements


typedef enum { // efectiv creezi un tip nou, aaa si valoarea variabilei e ordinea nice
    D9,
    D10,
    D11,
    D12,
    D13
}pin;

#endif 