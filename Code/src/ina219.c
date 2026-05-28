#include "ina219.h"
#include "i2cmaster.h"

//I2C write
static void INA219_writeRegister(uint8_t addr, uint8_t reg, uint16_t value)
{
    i2c_start(get_adress(addr, I2C_WRITE));
    i2c_write(reg); //tell the chip which register pointer we want to modify
    //splits the 16-bit register value into high/low bytes
    //INA219 expects the high byte first over the I2C line
    i2c_write_16bit_reg((value & 0xFF), ((value >> 8) & 0xFF));
    i2c_stop();
}

//I2C read
static uint16_t INA219_readRegister(uint8_t addr, uint8_t reg)
{
    uint16_t value;
    i2c_start(get_adress(addr, I2C_WRITE));
    i2c_write(reg); //point to the register we want to read from
    //restart I2C bus in read mode to pull data
    i2c_rep_start(get_adress(addr, I2C_READ));
    //transmits the high byte first, then the low byte
    value = ((uint16_t)i2c_readAck()) << 8; //read high byte and shift it to upper 8 bits
    value |= i2c_readNak();                 //read low byte and merge it, nak signals end of read
    i2c_stop();
    return value;
}

//initialization (takes custom config_value for different PGA modes)
void INA219_init(uint8_t addr, uint16_t config_value)
{
    //write to configuration register (0x00)
    //control board uses 0x39FF (PGA/1, max resolution for tiny motor drop)
    //-------- test this --------
    //generator board uses 0x3FFF (PGA/8, expands range to 320mV so 1 Ohm shunt wont clip)
    //both modes have 128x internal hardware averaging enabled to kill high-frequency motor noise
    INA219_writeRegister(addr, 0x00, config_value);
}

//read main supply voltage
float INA219_getBusVoltage(uint8_t addr)
{
    uint16_t raw;
    //read bus voltage register (0x02)
    raw = INA219_readRegister(addr, 0x02);
    //bits 0, 1, and 2 are internal status flags (like conversion ready)
    //shift bits right by 3 to throw away the flags and get clean data bits
    raw >>= 3;
    //the INA219 has a fixed bus resolution where 1 LSB = 4mV (0.004V)
    //multiplying converts the raw binary integer into a real float in Volts
    return raw * 0.004;
}

//read pure raw data from the shunt register
float INA219_getShuntVoltage(uint8_t addr)
{
    //read shunt voltage register (0x01)
    //cast to signed int16_t because current can flow backward through the shunt
    //returns the raw unshifted code so we can process it with the correct multipliers in main.c
    return (int16_t)INA219_readRegister(addr, 0x01);
}