#ifndef INA219_H_
#define INA219_H_

#include <stdint.h>
#include "i2cmaster.h"
#include "constants.h"

/* =========================
   INA219 Register Addresses
   ========================= */
typedef enum {
    CONFIGURATION = 0x00,
    SHUNT_VOLTAGE = 0x01,
    BUS_VOLTAGE   = 0x02,
    POWER         = 0x03,
    CURRENT       = 0x04,
    CALIBRATION   = 0x05
} register_address;


/* =========================
   Configuration Register Control
   =========================
   16V range
   Gain selected
   128 sample averaging
   Continuous shunt + bus mode
*/
#define INA219_CONTROL_CONFIG_REG_R ((uint8_t)0x07)
#define INA219_CONTROL_CONFIG_REG_L ((uint8_t)0xFF)

#define INA219_CONTROL_CALIB_REG_R ((uint8_t)0x80)
#define INA219_CONTROL_CALIB_REG_L ((uint8_t)0x00)

/* =========================
   Configuration Register Generator 
   =========================
   32V range
   Gain selected
   128 sample averaging
   Continuous shunt + bus mode
*/
#define INA219_GENERATOR_CONFIG_REG_R ((uint8_t)0x37)
#define INA219_GENERATOR_CONFIG_REG_L ((uint8_t)0xFF)

#define INA219_GENERATOR_CALIB_REG_R ((uint8_t)0x20)
#define INA219_GENERATOR_CALIB_REG_L ((uint8_t)0x00)

/* =========================
   Conversion Constants
   =========================
   Everything in micro-units, for LSB_CURRENT values were rounded up to get cleaner results (the CALIB doesnt need to be truncated
    and its simpler for the chip to avoid float points)
*/
#define LSB_BV                4000.0f     // 4mV = 4000uV
#define LSB_CURRENT_CONTROL   50 // 45.776uA per bit
#define LSB_CURRENT_GENERATOR 4 // 3.606uA per bit 

/* =========================
   Function Prototypes
   ========================= */

void INA219_INIT_CONTROL(void);
void INA219_INIT_GENERATOR(void);

float GET_CURRENT_CONTROL(void);
float GET_CURRENT_GENERATOR(void);

float GET_BUS_VOLTAGE(SLAVE_ADDRESS);

void GET_INA219_REG(SLAVE_ADDRESS,register_address);
#endif /* INA219_H_ */

/*Configuration Reg: 0x00
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
