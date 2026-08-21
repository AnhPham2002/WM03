#pragma once

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_pwr.h"
#include "stm32l4xx_hal_pwr_ex.h"
#include <stdint.h>

static inline void sys_delay_ms(uint32_t u32Delay)
{
    HAL_Delay(u32Delay);
}

static inline uint32_t sys_time_ms(void)
{
    return HAL_GetTick();
}

static inline void sys_sleep(void)
{
    HAL_SuspendTick();
    HAL_PWREx_EnterSTOP2Mode(PWR_STOPENTRY_WFI);
    HAL_ResumeTick();
}