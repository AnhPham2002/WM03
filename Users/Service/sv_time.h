#pragma once

#include "drv_rtc.h"

/**
 * @brief Initialize RTC.
 */
void sv_time_init(void);

/**
 * @brief Set date and time.
 *
 * @param[in] pDateTime Date and time to set.
 *
 * @return true if successful, otherwise false.
 */
bool sv_time_set_date_time(const Date_Time_t *pDateTime);

/**
 * @brief Get date and time.
 *
 * @param[out] pDateTime Current date and time.
 */
void sv_time_get_date_time(Date_Time_t *pDateTime);

/**
 * @brief Get the current Unix timestamp.
 *
 * Converts the current date and time to the number of seconds elapsed
 * since 00:00:00 UTC on January 1, 1970.
 *
 * @return Current Unix timestamp.
 */
uint32_t sv_time_get_unix_timestamp(void);

/**
 * @brief Get the system uptime in seconds.
 *
 * @return System uptime in seconds.
 */
uint32_t sv_time_get_uptime_seconds(void);

/**
 * @brief Get the elapsed time since power-on in seconds.
 *
 * @return Elapsed time since power-on in seconds.
 */
uint32_t sv_time_get_power_on_seconds(void);

/**
 * @brief Check whether date and time needs to be updated.
 *
 * @return true if the date and time is invalid or synchronization is requested; otherwise false.
 */
bool sv_time_get_date_time_update_required(void);