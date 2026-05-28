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
#define INA219_CONTROL_CONFIG_REG_L ((uint8_t)0x77)
#define SHUNT_R_CONTROL 0.025f

/* =========================
   Configuration Register Generator 
   =========================
   32V range
   Gain selected
   128 sample averaging
   Continuous shunt + bus mode
*/
#define INA219_GENERATOR_CONFIG_REG_R ((uint8_t)0x3F)
#define INA219_GENERATOR_CONFIG_REG_L ((uint8_t)0x77)
#define SHUNT_R_GENERATOR 1.0f

/* =========================
   Conversion Constants
   =========================
   Everything in micro-units, for LSB_CURRENT values were rounded up to get cleaner results (the CALIB doesnt need to be truncated
    and its simpler for the chip to avoid float points)
*/
#define LSB_BV                4000.0f     // 4mV = 4000uV
#define LSB_SHUNT               10.0f // 10uV

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

