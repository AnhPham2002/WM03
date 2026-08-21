#pragma once

#include "drv_gpio.h"
#include "drv_i2c.h"
#include "sys_common.h"
#include "sys_debug.h"

#define PWR_EXT_ADC_PIN GPIOB, GPIO_PIN_5
#define PWR_SS_PIN GPIOB, GPIO_PIN_3
#define EN_SS1_PIN GPIOD, GPIO_PIN_2
#define EN_SS2_PIN GPIOC, GPIO_PIN_12

#define EXT_ADC_CONVERSION_TIMEOUT 300

/* MCP3421 ADC Info */
#define EXT_ADC_ADDR (0x69 << 1) // MCP3421A1 I2C address 7-bit
#define EXT_ADC_VREF 2.048

/* MCP3421 Configuration Register */
// Bit mask
#define EXT_ADC_CR_G_POS 0 // PGA Gain Selection bits position
#define EXT_ADC_CR_G_MASK (3U << EXT_ADC_CR_G_POS)
#define EXT_ADC_CR_S_POS 2 // Sample Rate Selection bits position
#define EXT_ADC_CR_S_MASK (3U << EXT_ADC_CR_S_POS)
#define EXT_ADC_CR_MODE_POS 4 // Conversion Mode bis position
#define EXT_ADC_CR_MODE_MASK (1U << EXT_ADC_CR_MODE_POS)
#define EXT_ADC_CR_RDY_POS 7 // Ready bit position
#define EXT_ADC_CR_RDY_MASK (1U << EXT_ADC_CR_RDY_POS)
// PGA Gain Selection Bits value
#define EXT_ADC_CR_G_X1 0x00
#define EXT_ADC_CR_G_X2 0x01
#define EXT_ADC_CR_G_X4 0x02
#define EXT_ADC_CR_G_X8 0x03
// Sample Rate Selection Bit
#define EXT_ADC_CR_S_12BIT 0x00 // 240 SPS (12 bits)
#define EXT_ADC_CR_S_14BIT 0x01 // 60 SPS (14 bits)
#define EXT_ADC_CR_S_16BIT 0x02 // 15 SPS (16 bits)
#define EXT_ADC_CR_S_18BIT 0x03 // 3.75 SPS (18 bits)
// Conversion Mode Bit
#define EXT_ADC_CR_MODE_ONE_SHOT 0x00
#define EXT_ADC_CR_MODE_CONTINUOUS 0x02
// Ready Bit
#define EXT_ADC_CR_RDY_DIS 0x00
#define EXT_ADC_CR_RDY_EN 0x01

/**
 * @brief Turn on external ADC power.
 */
void drv_ext_adc_pwr_on(void);

/**
 * @brief Turn off external ADC power.
 */
void drv_ext_adc_pwr_off(void);

/**
 * @brief Initialize external ADC.
 */
void drv_ext_adc_init(void);

/**
 * @brief Enable sensor 1.
 */
void drv_ext_adc_enable_sensor1(void);

/**
 * @brief Enable sensor 2.
 */
void drv_ext_adc_enable_sensor2(void);

/**
 * @brief Disable all sensors.
 */
void drv_ext_adc_disable_all_sensors(void);

/**
 * @brief Read voltage from external ADC.
 *
 * @param[out] fVal Pointer to the converted voltage value in volts.
 *
 * @return true if ADC data is read successfully, otherwise false.
 */
bool drv_ext_adc_read(float *fVal);