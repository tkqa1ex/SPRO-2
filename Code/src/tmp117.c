#include "tmp117.h"
#include "i2cmaster.h"

//I2C write
static void TMP117_writeRegister(uint8_t addr, uint8_t reg, uint16_t value)
{
    i2c_start(get_adress(addr, I2C_WRITE));
    i2c_write(reg); //tell the chip which register pointer we want to modify
    //splits the 16-bit register value into high/low bytes
    //TMP117 expects the high byte first over the I2C line
    i2c_write_16bit_reg((value & 0xFF), ((value >> 8) & 0xFF));
    i2c_stop();
}

//I2C read
static uint16_t TMP117_readRegister(uint8_t addr, uint8_t reg)
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

//initialization
void TMP117_init(uint8_t addr)
{
    //write to configuration register (0x01) -> value: 0x0220
    //binary: 0000 0010 0010 0000
    //what this configures:
    // continuous conversion mode (chip samples non-stop)
    // 8x internal hardware averaging enabled (cleans up noise from the motor test stand)
    // 15.625ms conversion time
    TMP117_writeRegister(addr, 0x01, 0x0220);
}

//read temperature in celsius
float TMP117_readTemperature(uint8_t addr)
{
    int16_t raw;
    //read temperature result register (0x00)
    //cast to signed int16_t because temperature can drop below 0°C (Two's Complement)
    raw = (int16_t)TMP117_readRegister(addr, 0x00);
    //the TMP117 has a fixed data resolution where 1 LSB = 0.0078125 °C.
    //this is the decimal equivalent of 1 / 128.
    //multiplying the raw binary integer by this converts it into a real float in celsius.
    return raw * 0.0078125;
}