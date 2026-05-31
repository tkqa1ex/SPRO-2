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
void TMP117_INIT(uint8_t addr)
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