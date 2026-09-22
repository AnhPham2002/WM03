#pragma once

#include "app_cellular.h"
#include "app_latch.h"

#define PUSH_RETRY 3
#define PUSH_RESPONSE_TIMOUT 5000

typedef enum
{
    PUSH_STEP_IDLE = 0,
    PUSH_STEP_SEND_INFO,
    // PUSH_STEP_SEND_MODULE_CONFIG,
    // PUSH_STEP_SEND_PULSE_METER_CONFIG,
    // PUSH_STEP_SEND_MODBUS_METER_CONFIG,
    // PUSH_STEP_SEND_PRESSURE_SENSOR_CONFIG,
    PUSH_STEP_SEND_LATCH,
    PUSH_STEP_SEND_EVENT,
    PUSH_STEP_UPDATE_METADATA
} Push_Step_t;

/**
 * @brief Execute the data push process.
 *
 * Manages the cellular connection and sequentially pushes device information,
 * latch data and event data to the server. Updates the storage load indices
 * after successful transmission or when the retry limit is reached.
 */
void app_push_execute(void);