#pragma once

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_pwr.h"
#include "stm32l4xx_hal_pwr_ex.h"

#include "flash_layout.h"
#include "ram_noinit_layout.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define CLAMP(x, l, h) ((x) < (l) ? (l) : ((x) > (h) ? (h) : (x)))

static inline void sys_delay_ms(uint32_t u32Delay)
{
    HAL_Delay(u32Delay);
}

static inline uint32_t sys_time_ms(void)
{
    return HAL_GetTick();
}

uint16_t sys_crc16(const uint8_t *pData, uint16_t u16Size);
void sys_sleep(void);