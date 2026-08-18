#pragma once

#include "stm32l4xx_hal.h"
#include <stdint.h>

static inline void sys_delay_ms(uint32_t u32Delay)
{
    HAL_Delay(u32Delay);
}

static inline uint32_t sys_time_ms(void)
{
    return HAL_GetTick();
}