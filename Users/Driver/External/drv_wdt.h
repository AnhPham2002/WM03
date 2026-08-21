#pragma once

#include "setting.h"

#define WDI_PIN GPIOC, GPIO_PIN_13

#define WDT_MAX_REFRESH_COUNT 10

/**
 * @brief Initialize watchdog timer.
 */
void drv_wdt_init(void);

/**
 * @brief Refesh watchdog timer.
 */
void drv_wdt_refresh(void);

/**
 * @brief Clear refresh counter for watchdog timer.
 */
void drv_wdt_clear_refresh_count(void);