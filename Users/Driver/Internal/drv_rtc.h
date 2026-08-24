#pragma once

#include "drv_wdt.h"
#include "drv_led.h"

#include <stdint.h>
#include <stdbool.h>

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
 * @brief Check whether the RTC date and time needs to be updated.
 *
 * @return true if the date and time is invalid or synchronization is requested; otherwise false.
 */
bool drv_rtc_is_date_time_update_required(void);