#pragma once

#include "drv_led.h"
#include "drv_wdt.h"


#include <stdbool.h>
#include <stdint.h>


#define MINUTES_PER_DAY 1440

typedef struct
{
    uint8_t u8Year;
    uint8_t u8Month;
    uint8_t u8Date;
    uint8_t u8Hours;
    uint8_t u8Minutes;
    uint8_t u8Seconds;
} Date_Time_t;

/**
 * @brief Initialize RTC.
 */
void drv_rtc_init(void);

/**
 * @brief Set RTC date and time.
 *
 * @param[in] pDateTime Date and time to set.
 *
 * @return true if successful, otherwise false.
 */
bool drv_rtc_set_date_time(const Date_Time_t *pDateTime);

/**
 * @brief Get RTC date and time.
 *
 * @param[out] pDateTime Current date and time.
 */
void drv_rtc_get_date_time(Date_Time_t *pDateTime);

/**
 * @brief Get the system uptime in seconds.
 *
 * @return System uptime in seconds.
 */
uint32_t drv_rtc_get_uptime_seconds(void);

/**
 * @brief Get the elapsed time since power-on in seconds.
 *
 * @return Elapsed time since power-on in seconds.
 */
uint32_t drv_rtc_get_power_on_seconds(void);

/**
 * @brief Check whether the RTC date and time needs to be updated.
 *
 * @return true if the date and time is invalid or synchronization is requested; otherwise false.
 */
bool drv_rtc_is_date_time_update_required(void);