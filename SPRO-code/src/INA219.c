#include <avr/io.h>
#include <stdio.h>
#include <stdbool.h>
#include "i2cmaster.h"
#include "constants.h"
#include "INA219.h"



void INA219_INIT_CONTROL() {
    // configuration reg
    i2c_start_wait(get_adress(INA219_CONTROL,0));
    i2c_write(CONFIGURATION);
    i2c_write(INA219_CONTROL_CONFIG_REG_R);
    i2c_write(INA219_CONTROL_CONFIG_REG_L);

    //i2c_write_16bit_reg(INA219_CONTROL_CONFIG_REG_R,INA219_CONTROL_CONFIG_REG_L); 

    // // calibration reg
    // i2c_rep_start(get_adress(INA219_CONTROL,0));
    // i2c_write(CALIBRATION);
    // i2c_write(INA219_CONTROL_CALIB_REG_R);
    // i2c_write(INA219_CONTROL_CALIB_REG_L);

    i2c_stop();
}

void INA219_INIT_GENERATOR() {
    // configuration reg
    // we get big Voltage readings so we'll assume max voltage 18-25 < 160mV -> GAIN/2
    i2c_start_wait(get_adress(INA219_GENERATOR,0));
    i2c_write(CONFIGURATION); 
    i2c_write(INA219_GENERATOR_CONFIG_REG_R);
    i2c_write(INA219_GENERATOR_CONFIG_REG_L);

    // // calibration reg
    // i2c_rep_start(get_adress(INA219_GENERATOR,0));
    // i2c_write(CALIBRATION);
    // i2c_write(INA219_GENERATOR_CALIB_REG_R);
    // i2c_write(INA219_GENERATOR_CALIB_REG_L);

    i2c_stop();
}

void GET_INA219_REG(SLAVE_ADDRESS slave_address,register_address reg) {
    i2c_start_wait(get_adress(slave_address,0));
    i2c_write(reg);
    
    i2c_rep_start(get_adress(slave_address,1));
    uint8_t bus_R = i2c_readAck();
    uint8_t bus_L = i2c_readNak();
    i2c_stop();
    printf("%x%x\n",bus_R,bus_L);
}

float GET_CURRENT_CONTROL() {
    i2c_start_wait(get_adress(INA219_CONTROL,0));
    i2c_write(SHUNT_VOLTAGE);

    i2c_rep_start(get_adress(INA219_CONTROL,1));
    int8_t shunt_R = i2c_readAck();
    int8_t shunt_L = i2c_readNak();
    i2c_stop();
    int16_t shunt_voltage = ((int16_t)shunt_R << 8) + shunt_L;
    float value = shunt_voltage * LSB_SHUNT / SHUNT_R_CONTROL;
    value *= 0.001; // in mA
    return value;
}

float GET_CURRENT_GENERATOR() {
    i2c_start_wait(get_adress(INA219_GENERATOR,0));
    i2c_write(SHUNT_VOLTAGE);

    i2c_rep_start(get_adress(INA219_GENERATOR,1));
    int8_t shunt_R = i2c_readAck();
    int8_t shunt_L = i2c_readNak();
    i2c_stop();
    int16_t shunt_voltage = ((int16_t)shunt_R << 8) + shunt_L;
    // printf("%" PRIu16"\n",current);
    float value = shunt_voltage * LSB_SHUNT / SHUNT_R_GENERATOR;
    value *= 0.001; // in mA
    return value;
}

float GET_BUS_VOLTAGE(SLAVE_ADDRESS slave) {
    i2c_start_wait(get_adress(slave,0));
    i2c_write(BUS_VOLTAGE);

    
    i2c_rep_start(get_adress(slave,1));
    uint8_t bus_R = i2c_readAck();
    uint8_t bus_L = i2c_readNak();
    i2c_stop();
    uint16_t bus = (bus_R << 8) + bus_L;
    bus >>= 3; // needs to be shifted right by 3 bits

    float bus_voltage = bus * LSB_BV;
    bus_voltage *= 0.000001f;
    return bus_voltage;
}

/*
Interesant modul in care se calculeaza la shunt
Ai valoarea reala, care in functie de gain, accuracy se modifica in cea din registru
basically: SHUNT_REG = V_SHUNT / LSB_V


*/
