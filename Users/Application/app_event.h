#pragma once

#include "app_storage.h"
#include "sv_485_communication.h"

#define MAGNETIC_DETECT_MIN_TIME_SECOND 3
#define MAGNETIC_END_DELAY_SECOND 600
#define POWER_STATUS_CONFIRM_SECOND 5
#define BATTERY_LOW_THRESHOLD_MV 3200
#define BATTERY_LOW_RECOVERY_MV 3400
#define BATTERY_STATUS_CHECK_SECOND 30

/**
 * @brief Execute event monitoring and recording.
 */
void app_event_execute(void);
