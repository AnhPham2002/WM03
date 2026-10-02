#pragma once

#include "drv_charge.h"

/**
 * @brief Initialize charger service.
 *
 * Configures the input current limit, charge voltage, fast charge current,
 * LED status, and charger watchdog.
 *
 * @param[in] u16IinLimMa Input current limit in mA.
 * @param[in] u16VoltageMv Charge voltage in mV.
 * @param[in] u16Current Fast charge current in mA.
 * @param[in] bLedStatus Charger LED status.
 */
void sv_charge_init(uint16_t u16IinLimMa, uint16_t u16VoltageMv, uint16_t u16Current, bool LedStatus);

/**
 * @brief Check whether a charger fault is detected.
 *
 * @return true if a charger fault is detected, otherwise false.
 */
bool sv_charge_fault_detected(void);

/**
 * @brief Enable charger configuration.
 *
 * @return true if the charger configuration is enabled successfully, otherwise false.
 */
bool sv_charge_config_enable(void);

/**
 * @brief Disable charger configuration.
 *
 * @return true if the charger configuration is disabled successfully, otherwise false.
 */
bool sv_charge_config_disable(void);

/**
 * @brief Set charger input current limit.
 *
 * @param[in] u16CurrentMa Input current limit in mA.
 *
 * @return true if the setting is successful, otherwise false.
 */
bool sv_charge_set_input_current_limit(uint16_t u16CurrentMa);

/**
 * @brief Start charger ADC conversion.
 *
 * @return true if the conversion is started successfully, otherwise false.
 */
bool sv_charge_start_adc_conversion(void);

/**
 * @brief Reset charger watchdog timer.
 *
 * @return true if the watchdog is reset successfully, otherwise false.
 */
bool sv_charge_wd_rst(void);

/**
 * @brief Set fast charge current.
 *
 * @param[in] u16CurrentMa Fast charge current in mA.
 *
 * @return true if the setting is successful, otherwise false.
 */
bool sv_charge_set_fast_charge_current(uint16_t u16CurrentMa);

/**
 * @brief Set charge voltage.
 *
 * @param[in] u16VoltageMv Charge voltage in mV.
 *
 * @return true if the setting is successful, otherwise false.
 */
bool sv_charge_set_charge_voltage(uint16_t u16VoltageMv);

/**
 * @brief Enable charger status LED.
 *
 * @return true if the setting is successful, otherwise false.
 */
bool sv_charge_turn_on_led_status(void);

/**
 * @brief Disable charger status LED.
 *
 * @return true if the setting is successful, otherwise false.
 */
bool sv_charge_turn_off_led_status(void);

/**
 * @brief Disconnect the battery.
 *
 * @return true if the battery is disconnected successfully, otherwise false.
 */
bool sv_charge_disconnect_battery(void);

/**
 * @brief Get VBUS status.
 *
 * @param[out] u8Status VBUS status.
 *
 * @return true if the status is read successfully, otherwise false.
 */
bool sv_charge_get_vbus_status(uint8_t *u8Status);

/**
 * @brief Get charging status.
 *
 * @param[out] u8Status Charging status.
 *
 * @return true if the status is read successfully, otherwise false.
 */
bool sv_charge_get_charging_status(uint8_t *u8Status);

/**
 * @brief Get charger fault status.
 *
 * @param[out] u8Fault Fault status.
 *
 * @return true if the fault status is read successfully, otherwise false.
 */
bool sv_charge_get_fault(uint8_t *u8Fault);

/**
 * @brief Get battery voltage.
 *
 * @param[out] u16VoltageMv Battery voltage in mV.
 *
 * @return true if the voltage is read successfully, otherwise false.
 */
bool sv_charge_get_battery_voltage(uint16_t *u16VoltageMv);