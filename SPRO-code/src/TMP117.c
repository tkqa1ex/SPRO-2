#include "tmp117.h"
#include "i2cmaster.h"

static void TMP117_writeRegister(uint8_t addr, uint8_t reg, uint16_t value)
{
    i2c_start((addr << 1) | I2C_WRITE);
    i2c_write(reg);
    i2c_write((value >> 8) & 0xFF);
    i2c_write(value & 0xFF);
    i2c_stop();
}

static uint16_t TMP117_readRegister(uint8_t addr, uint8_t reg)
{
    uint16_t value;
    i2c_start((addr << 1) | I2C_WRITE);
    i2c_write(reg);
    i2c_rep_start((addr << 1) | I2C_READ);
    value = ((uint16_t)i2c_readAck()) << 8;
    value |= i2c_readNak();
    i2c_stop();
    return value;
}

void TMP117_INIT(uint8_t addr)
{
    TMP117_writeRegister(addr, 0x01, 0x0220);
}

float TMP117_readTemperature(uint8_t addr)
{
    int16_t raw;
    raw = (int16_t)TMP117_readRegister(addr, 0x00);
    return raw * 0.0078125;
}
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