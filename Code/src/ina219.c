#include "ina219.h"
#include "i2cmaster.h"

static void INA219_writeRegister(uint8_t addr, uint8_t reg, uint16_t value)
{
    i2c_start(get_adress(addr, I2C_WRITE));
    i2c_write(reg);
    // Splits the 16-bit register value into high/low bytes using your custom function
    i2c_write_16bit_reg((value & 0xFF), ((value >> 8) & 0xFF));
    i2c_stop();
}

static uint16_t INA219_readRegister(uint8_t addr, uint8_t reg)
{
    uint16_t value;
    i2c_start(get_adress(addr, I2C_WRITE));
    i2c_write(reg);
    i2c_rep_start(get_adress(addr, I2C_READ));
    value = ((uint16_t)i2c_readAck()) << 8;
    value |= i2c_readNak();
    i2c_stop();
    return value;
}

void INA219_init(uint8_t addr)
{
    // Configured for PGA /4 (±160mV Range), 12-bit, Continuous -> 0x359F
    INA219_writeRegister(addr, 0x00, 0x359F);
}

float INA219_getBusVoltage(uint8_t addr)
{
    uint16_t raw;
    raw = INA219_readRegister(addr, 0x02);
    raw >>= 3;
    return raw * 0.004;
}

float INA219_getShuntVoltage(uint8_t addr)
{
    int16_t raw;
    raw = (int16_t)INA219_readRegister(addr, 0x01);
    
    // Multiplier for unshifted library register reads at PGA /4
    return raw * 0.005;
}