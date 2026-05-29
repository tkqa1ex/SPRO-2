#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>
#include "i2cmaster.h"
#include "constants.h"
#include "INA219.h"


// initialise the INA219 on the controller
void INA219_INIT_CONTROL() {
    // configuration reg
    i2c_start_wait(get_adress(INA219_CONTROL,0));
    // register to be accessed in reading mode
    i2c_write(CONFIGURATION);

    // sending the 2 byte data
    i2c_write(INA219_CONTROL_CONFIG_REG_R);
    i2c_write(INA219_CONTROL_CONFIG_REG_L);

    // stoping the communication
    i2c_stop();
}

// initialise the INA219 on the generator
void INA219_INIT_GENERATOR() {
    // configuration reg
    i2c_start_wait(get_adress(INA219_GENERATOR,0));
    // register to be accessed in reading mode
    i2c_write(CONFIGURATION); 

    // sending the 2 byte data
    i2c_write(INA219_GENERATOR_CONFIG_REG_R);
    i2c_write(INA219_GENERATOR_CONFIG_REG_L);

    // stoping the communication
    i2c_stop();
}


// generally used for debugging purposes
// to select the sensor to read from (slave_address) and the register address (reg)
void GET_INA219_REG(SLAVE_ADDRESS slave_address,register_address reg) {
    i2c_start_wait(get_adress(slave_address,0));
    // register to be accessed in reading mode
    i2c_write(reg);
    
    // start communication in reading mode
    i2c_rep_start(get_adress(slave_address,1));
    uint8_t bus_R = i2c_readAck();
    uint8_t bus_L = i2c_readNak();
    i2c_stop();
    printf("%x%x\n",bus_R,bus_L);
}

// get the current from thec control circuit
float GET_CURRENT_CONTROL() {
    // wake-up the IN219 sensor from the control board
    i2c_start_wait(get_adress(INA219_CONTROL,0));
    // acces the SHUNT VOLTAGE register
    i2c_write(SHUNT_VOLTAGE);

    // start communication in reading mode
    i2c_rep_start(get_adress(INA219_CONTROL,1));
    int8_t shunt_R = i2c_readAck();
    int8_t shunt_L = i2c_readNak();
    i2c_stop();
    // data recevied from the sensor
    int16_t shunt_voltage = ((int16_t)shunt_R << 8) + shunt_L;

    // use Ohms law to calculate the current = V / R
    // that value needs to be multipled by the resolution of the SHUNT reading (LSB SHUNT)
    float current = shunt_voltage / SHUNT_R_CONTROL * LSB_SHUNT;
    current *= 0.001; // in mA
    return current;
}

// get the current from the generator circuit
float GET_CURRENT_GENERATOR() {
    // wake up the INA219 from the generator
    i2c_start_wait(get_adress(INA219_GENERATOR,0));
    // register to be accessed in reading mode
    i2c_write(SHUNT_VOLTAGE);

    // start communication in reading mode
    i2c_rep_start(get_adress(INA219_GENERATOR,1));
    int8_t shunt_R = i2c_readAck();
    int8_t shunt_L = i2c_readNak();
    i2c_stop();
    // data recevied from the sensor
    int16_t shunt_voltage = ((int16_t)shunt_R << 8) + shunt_L;

    // use Ohms law to calculate the current = V / R
    // that value needs to be multipled by the resolution of the SHUNT reading (LSB SHUNT)
    float current = shunt_voltage / SHUNT_R_GENERATOR * LSB_SHUNT;
    current *= 0.001; // in mA
    return current;
}

// get the voltage across the motor/generator (depending on the slave that is called)
float GET_BUS_VOLTAGE(SLAVE_ADDRESS slave) {
    // wake-up the slave
    i2c_start_wait(get_adress(slave,0));
    // register to be accessed in reading mode
    i2c_write(BUS_VOLTAGE);

    
    // start communication in reading mode
    i2c_rep_start(get_adress(slave,1));
    uint8_t bus_R = i2c_readAck();
    uint8_t bus_L = i2c_readNak(); 
    // stop transmission
    i2c_stop();

    // data recevied from the sensor
    uint16_t bus = (bus_R << 8) + bus_L;
    bus >>= 3; // needs to be shifted right by 3 bits

    // multiply by the resolution for the BUS VOLTAGE(LSB_BV)
    float bus_voltage = bus * LSB_BV;
    bus_voltage *= 0.000001f;
    return bus_voltage;
}

/*
Configuration Reg: 0x00
    13 bit: BRNG - (bus voltage) set to 0 (0-16V)
    11-12 bit: Gain and Range settings (the bigger gain the better)
    collect data every 200ms => 5 times a s (altough IN219 can do every 70ms but 100ms to be sure) -> ADC4-1 = 0b1111
    MODE3-1: u want continuous (dont need to manually trigger) for current (shunt) and voltage (bus)
    it says that triggered is used to make it be synchronous (so u know when the new data is processed and dont accidentally use data)
    if during the changing voltage period we get points which are very much alike then we'll need triggered
    Control: 0b000 00 1111 1111 111
    16V, 40mV, needs 70ms to average 128 samples, shunt and bus continuous

    Generator: 0b001 10 1111 1111 111
    32V, 160mV, needs 70ms to average 128 samples, shunt and bus continuous
    
 Shunt Voltage: 0x01 -> stores the shunt reading -> 10uV
    Bus Voltage: 0x02 -> LSB -> 4mV
    Current Register: 0x04 -> Current_LSB
    The Bus Voltage register bits are not right-aligned. In order to compute the value of the Bus Voltage, Bus Voltage
    Register contents must be shifted right by three bits. This shift puts the BD0 bit in the LSB position so that the
    contents can be multiplied by the Bus Voltage LSB of 4-mV to compute the bus voltage measured by the device.

Calibration reg -> 0x05
    The Calibration Register is calculated based on Equation 1. This equation includes the term Current_LSB, which
    is the programmed value for the LSB for the Current Register (04h). The user uses this value to convert the
    value in the Current Register (04h) to the actual current in amperes.
    Eq 1: Cal = trunc(0.04096 * Current_LSB / R_SHUNT)
    Current_LSB (from eq2) = Imax/2^15
    So we just reverse engineer for our R_shunt and Imax what the reg should be equal to

    !!!!FOR CURRENT LSB: While this value yields the highest resolution (the literal formula), it is common to select a value for
the Current_LSB to the nearest round number above this value to simplify the conversion of the Current Register
(04h) and Power Register (03h) to amperes and watts respectively. 
*/