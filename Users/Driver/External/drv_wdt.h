#pragma once

#include "setting.h"

#define WDI_PIN GPIOC, GPIO_PIN_13

/**
 * @brief Initialize watchdog timer.
 */
void drv_wdt_init(void);

/**
 * @brief Restart watchdog timer.
 */
void drv_wdr_restart(void);