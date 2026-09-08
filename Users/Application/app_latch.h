#pragma once

#include "app_storage.h"

typedef enum
{
    LATCH_STEP_IDLE = 0,
    LATCH_STEP_READ_PULSE_METER,
    LATCH_STEP_READ_MODBUS_METER,
    LATCH_STEP_READ_PRESSURE_SENSOR,
    LATCH_STEP_SAVE_EEPROM
} Latch_Step_t;

/**
 * @brief Execute latch data acquisition process.
 *
 * Processes pulse meters, Modbus meters and pressure sensors sequentially,
 * then saves the collected latch data to EEPROM.
 */
void app_latch_execute(void);

/**
 * @brief Activate latch data acquisition.
 *
 * Sets the latch period flag to start the latch process.
 */
void app_latch_activate(void);