#pragma once

#include "app_storage.h"
#include "sv_485_communication.h"

#define MAGNETIC_DETECT_MIN_TIME_MS 2000
#define MAGNETIC_END_DELAY_MS 60000
#define POWER_STATUS_CONFIRM_TIME_MS 2000
#define BATTERY_LOW_THRESHOLD_MV 3200
#define BATTERY_LOW_RECOVERY_MV 3400
#define BATTERY_STATUS_CONFIRM_TIME_MS 2000

/**
 * @brief Execute event monitoring and recording.
 */
void app_event_execute(void);
