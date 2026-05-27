#include "tmp117.h"
#include "i2cmaster.h"

static void TMP117_writeRegister(uint8_t addr, uint8_t reg, uint16_t value)
{
    i2c_start(get_adress(addr, I2C_WRITE));
    i2c_write(reg);
    i2c_write_16bit_reg((value & 0xFF), ((value >> 8) & 0xFF));
    i2c_stop();
}

static uint16_t TMP117_readRegister(uint8_t addr, uint8_t reg)
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

void TMP117_init(uint8_t addr)
{
    TMP117_writeRegister(addr, 0x01, 0x0220);
}

float TMP117_readTemperature(uint8_t addr)
{
    int16_t raw;
    raw = (int16_t)TMP117_readRegister(addr, 0x00);
    return raw * 0.0078125;
}