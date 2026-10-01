#pragma once

#include "drv_gpio.h"
#include "drv_i2c.h"

#include "sys_common.h"

#define CHG_EN_PIN GPIOC, GPIO_PIN_0
#define CHG_INT_PIN GPIOC, GPIO_PIN_13

/* BQ25895 Info */
#define CHG_TARGET_ADDR (0x6A << 1) // BQ25895 I2C address 7-bit

/* BQ25895 Register */
#define CHG_REG00 0x00
#define CHG_REG01 0x01
#define CHG_REG02 0x02
#define CHG_REG03 0x03
#define CHG_REG04 0x04
#define CHG_REG05 0x05
#define CHG_REG06 0x06
#define CHG_REG07 0x07
#define CHG_REG08 0x08
#define CHG_REG09 0x09
#define CHG_REG0A 0x0A
#define CHG_REG0B 0x0B
#define CHG_REG0C 0x0C
#define CHG_REG0D 0x0D
#define CHG_REG0E 0x0E
#define CHG_REG0F 0x0F
#define CHG_REG10 0x10
#define CHG_REG11 0x11
#define CHG_REG12 0x12
#define CHG_REG13 0x13
#define CHG_REG14 0x14

/* REG00 Register */
// Bit mask
#define CHG_REG00_EN_HIZ_POS 7 // Enable HIZ Mode Position
#define CHG_REG00_EN_HIZ_MASK (1U << CHG_REG00_EN_HIZ_POS)
#define CHG_REG00_EN_ILIM_POS 6 // Enable ILIM Pin Position
#define CHG_REG00_EN_ILIM_MASK (1U << CHG_REG00_EN_ILIM_POS)
#define CHG_REG00_IINLIM_POS 0 // Input Current Limit Position
#define CHG_REG00_IINLIM_MASK (63U << CHG_REG00_IINLIM_POS)
// Input Current Limit Range
#define CHG_IINLIM_MIN_MA 100
#define CHG_IINLIM_MAX_MA 3250
#define CHG_IINLIM_OFFSET_MA 100
#define CHG_IINLIM_STEP_MA 50

/* REG02 Register */
#define CHG_REG02_CONV_START_POS 7 // ADC Conversion Start Control Position
#define CHG_REG02_CONV_START_MASK (1U << CHG_REG02_CONV_START_POS)
#define CHG_REG02_CONV_RATE_POS 6 // ADC Conversion Rate Selection Position
#define CHG_REG02_CONV_RATE_MASK (1U << CHG_REG02_CONV_RATE_POS)
// ADC Conversion Start Control Bit
#define CHG_REG02_CONV_START_NOT_ACTIVE 0x00 // ADC conversion not active
#define CHG_REG02_CONV_START_ACTIVE 0x01     //  Start ADC Conversion when one-shot mode (continuous mode is ignored)
// ADC Conversion Rate Selection Bit
#define CHG_REG02_CONV_RATE_ONE_SHOT 0x00   // One shot ADC conversion
#define CHG_REG02_CONV_RATE_CONTINUOUS 0x01 // Start 1s Continuous Conversion

/* REG03 Register */
#define CHG_REG03_WD_RST_POS 6 // I2C Watchdog Timer Reset Position
#define CHG_REG03_WD_RST_MASK (1U << CHG_REG03_WD_RST_POS)
#define CHG_REG03_CHG_CONFIG_POS 4 // Charge Enable Configuration Position
#define CHG_REG03_CHG_CONFIG_MASK (1U << CHG_REG03_CHG_CONFIG_POS)
// I2C Watchdog Timer Reset Bit
#define CHG_REG03_WD_RST_NORMAL 0x00
#define CHG_REG03_WD_RST_RESET 0x01
// Charge Enable Configuration Bit
#define CHG_REG03_CHG_DIS 0x00 // Charge Disable
#define CHG_REG03_CHG_EN 0x01 // Charge Enable

/* REG04 Register */
#define CHG_REG04_EN_PUMPX_POS 7 // Current pulse control Enable Position
#define CHG_REG04_EN_PUMPX_MASK (1U << CHG_REG04_EN_PUMPX_POS)
#define CHG_REG04_ICHG_POS 0 // Fast Charge Current Limit Position
#define CHG_REG04_ICHG_MASK (127U << CHG_REG04_ICHG_POS)
// Current pulse control Enable Bit
#define CHG_REG04_PUMPX_DIS 0x00 // Disable Current pulse control
#define CHG_REG04_PUMPX_EN 0x01  // Enable Current pulse control
// Fast Charge Current Limit Range
#define CHG_ICHG_MIN_MA 0
#define CHG_ICHG_MAX_MA 5056
#define CHG_ICHG_OFFSET_MA 0
#define CHG_ICHG_STEP_MA 64

/* REG06 Register */
#define CHG_REG06_VREG_POS 2 // Charge Voltage Limit Position
#define CHG_REG06_VREG_MASK (63U << CHG_REG06_VREG_POS)
// Charge Voltage Limit Range
#define CHG_VREG_MIN_MV 3840
#define CHG_VREG_MAX_MV 4608
#define CHG_VREG_OFFSET_MV 3840
#define CHG_VREG_STEP_MV 16

/* REG07 Register */
#define CHG_REG07_STAT_DIS_POS 6 // STAT Pin Disable Position
#define CHG_REG07_STAT_DIS_MASK (1U << CHG_REG07_STAT_DIS_POS)
// STAT Pin Disable Bit
#define CHG_REG07_STAT_EN 0x00 // Enable STAT pin function
#define CHG_REG07_STAT_DIS 0x01 // Disable STAT pin function

/* REG09 Register */
#define CHG_REG09_BATFET_DIS_POS 5 // Force BATFET off to enable ship mode Position
#define CHG_REG09_BATFET_DIS_MASK (1U << CHG_REG09_BATFET_DIS_POS)
// Force BATFET off to enable ship mode Bit
#define CHG_REG09_BATFET_ON 0x00 // Allow BATFET turn on
#define CHG_REG09_BATFET_OFF 0x01 // Force BATFET off

/* REG0B Register */
#define CHG_REG0B_VBUS_STAT_POS 5 // VBUS Status Position
#define CHG_REG0B_VBUS_STAT_MASK (7U << CHG_REG0B_VBUS_STAT_POS)
#define CHG_REG0B_CHRG_STAT_POS 3 // Charging Status Position
#define CHG_REG0B_CHRG_STAT_MASK (3U << CHG_REG0B_CHRG_STAT_POS)
#define CHG_REG0B_PG_STAT_POS 2 // Power Good Status Position
#define CHG_REG0B_PG_STAT_MASK (1U << CHG_REG0B_PG_STAT_POS)
#define CHG_REG0B_SDP_STAT_POS 1 // USB Input Status Position
#define CHG_REG0B_SDP_STAT_MASK (1U << CHG_REG0B_SDP_STAT_POS)
#define CHG_REG0B_VSYS_STAT_POS 0 // VSYS Regulation Status Position
#define CHG_REG0B_VSYS_STAT_MASK (1U << CHG_REG0B_VSYS_STAT_POS)
// VBUS Status register Bits
#define CHG_VBUS_STAT_NO_INPUT 0x00
#define CHG_VBUS_STAT_USB_SDP 0x01
#define CHG_VBUS_STAT_USB_CDP 0x02
#define CHG_VBUS_STAT_USB_DCP 0x03
#define CHG_VBUS_STAT_ADJ_HV_DCP 0x04
#define CHG_VBUS_STAT_UNKNOW_ADAPTER 0x05
#define CHG_VBUS_STAT_NON_STANDARD_ADAPTER 0x06
#define CHG_VBUS_STAT_OTG 0x07
// Charging Status Bits
#define CHG_CHRG_STAT_NOT_CHARGING 0x00
#define CHG_CHRG_STAT_PRE_CHARGE 0x01
#define CHG_CHRG_STAT_FAST_CHARGING 0x02
#define CHG_CHRG_STAT_CHARGE_TERMINATION_DONE 0x03
// Power Good Status Bit
#define CHG_PG_STAT_NOT_GOOD 0x00
#define CHG_PG_STAT_GOOD 0x01
// USB Input Status
#define CHG_SDP_STAT_USB100 0x00
#define CHG_SDP_STAT_USB500 0x01
// VSYS Regulation Status Bit
#define CHG_VSYS_STAT_NOT_IN_REGULATION 0x00
#define CHG_VSYS_STAT_IN_REGULATION 0x01

/* REG0C Register */
#define CHG_REG0C_WATCHDOG_FAULT_POS 7 // Watchdog Fault Status Position
#define CHG_REG0C_WATCHDOG_FAULT_MASK (1U << CHG_REG0C_WATCHDOG_FAULT_POS)
#define CHG_REG0C_BOOST_FAULT_POS 6 // Boost Mode Fault Status Position
#define CHG_REG0C_BOOST_FAULT_MASK (1U << CHG_REG0C_BOOST_FAULT_POS)
#define CHG_REG0C_CHRG_FAULT_POS 4 // Charge Fault Status Position
#define CHG_REG0C_CHRG_FAULT_MASK (3U << CHG_REG0C_CHRG_FAULT_POS)
#define CHG_REG0C_BAT_FAULT_POS 3 // Battery Fault Status Position
#define CHG_REG0C_BAT_FAULT_MASK (1U << CHG_REG0C_BAT_FAULT_POS)
#define CHG_REG0C_NTC_FAULT_POS 0 // NTC Fault Status Position
#define CHG_REG0C_NTC_FAULT_MASK (7U << CHG_REG0C_NTC_FAULT_POS)
// Watchdog Fault Status Bit
#define CHG_WATCHDOG_NORMAL 0x00
#define CHG_WATCHDOG_TIMER_EXPIRATION 0x01
// Boost Mode Fault Status Bit
#define CHG_BOOST_NOMAL 0x00
#define CHG_BOOST_FAULT 0x01
// Charge Fault Status Bits
#define CHG_CHRG_NOMAL 0x00
#define CHG_CHRG_INPUT_FAULT 0x01
#define CHG_CHRG_THERMAL_SHUTDOWN 0x02
#define CHG_CHRG_SAFETY_TIMER_EXPIRATION 0x03
// Battery Fault Status Bit
#define CHG_BAT_NORMAL 0x00
#define CHG_BAT_OVP 0x01
// NTC Fault Status Bits
#define CHG_NTC_NORMAL 0x00
#define CHG_NTC_BUCK_COLD 0x01
#define CHG_NTC_BUCK_HOT 0x02
#define CHG_NTC_BOOST_COLD 0x05
#define CHG_NTC_BOOST_HOT 0x06

/* REG0E Register */
#define CHG_REG0E_THERM_STAT_POS 7 // Thermal Regulation Status Position
#define CHG_REG0E_THERM_STAT_MASK (1U << CHG_REG0E_THERM_STAT_POS)
#define CHG_REG0E_BATV_POS 0 // ADC conversion of Battery Voltage Position
#define CHG_REG0E_BATV_MASK (127U << CHG_REG0E_BATV_POS)
// Thermal Regulation Status Bit
#define CHG_THERM_NOMAL 0x00
#define CHG_THERM_REGULATION 0x01
// Battery volatage
#define CHG_BATV_MIN_MV 2304
#define CHG_BATV_MAX_MV 4848
#define CHG_BATV_OFFSET_MV 2304
#define CHG_BATV_STEP_MV 20


/**
 * @brief Initialize the charger driver.
 */
void drv_charge_init(void);

/**
 * @brief Enable battery charger.
 */
void drv_charge_enable(void);

/**
 * @brief Disable battery charger.
 */
void drv_charge_disable(void);

/**
 * @brief Write data to a charger register.
 *
 * @param[in] u8RegAddr Register address.
 * @param[in] u8Data    Register data.
 *
 * @return true if the data is written successfully, otherwise false.
 */
bool drv_charge_write(uint8_t u8RegAddr, uint8_t u8Data);

/**
 * @brief Read data from a charger register.
 *
 * @param[in]  u8RegAddr Register address.
 * @param[out] u8Data    Register data.
 *
 * @return true if the data is read successfully, otherwise false.
 */
bool drv_charge_read(uint8_t u8RegAddr, uint8_t *u8Data);

/**
 * @brief Handle charger fault interrupt.
 */
void drv_charge_interrupt(void);

/**
 * @brief Check whether a charger fault is detected.
 *
 * Clears the fault flag after reading it.
 *
 * @return true if a charger fault is detected, otherwise false.
 */
bool drv_charge_fault_detected(void);