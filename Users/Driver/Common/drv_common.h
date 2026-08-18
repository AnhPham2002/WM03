#pragma once

#include "stm32l4xx_hal.h"

static inline void delay_ms(uint32_t u32Delay)
{
    HAL_Delay(u32Delay);
}