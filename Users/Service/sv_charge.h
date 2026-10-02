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
 * @brief Check whether a charger interrupt is detected.
 *
 * @return true if a charger interrupt is detected, otherwise false.
 */
bool sv_charge_interrupt_detected(void);

/**
 * @brief Set charger input current limit.
 *
 * @param[in] u16CurrentMa Input current limit in mA.
 *
 * @return true if the setting is successful, otherwise false.
 */
bool sv_charge_set_input_current_limit(uint16_t u16CurrentMa);

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
 * @brief Get charger fault status.
 *
 * @param[out] u8Fault Fault status.
 *
 * @return true if the fault status is read successfully, otherwise false.
 */
bool sv_charge_get_fault(uint8_t *u8Fault);

/**
 * @brief Get charger power information.
 *
 * Gets the VBUS status, charging status, and battery voltage.
 *
 * @param[out] bVbusStatus        VBUS status.
 * @param[out] u8ChargingStatus   Charging status.
 * @param[out] u16BatteryVoltageMv Battery voltage in mV.
 */
void sv_charge_get_power_info(bool *bVbusStatus, uint8_t *u8ChargingStatus, uint16_t *u16BatteryVoltageMv);
