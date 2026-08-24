#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "stm32l4xx_hal.h"

#include "sys_common.h"

#include "drv_led.h"

#define LPTIM1_CLOCK_SOURCE 32 // kHz

/**
 * @brief Initialize Timer1 for low-power operation.
 *
 * @param[in] u32Period Timer period.
 */
void drv_timer1_low_power_init(uint32_t u32Period);