#pragma once

#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_pwr.h"
#include "stm32l4xx_hal_pwr_ex.h"

#include "eeprom_layout.h"
#include "flash_layout.h"
#include "ram_noinit_layout.h"

#include "sys_debug.h"

#include <math.h>
#include <stdbool.h>
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

static inline void sys_reset(void)
{
    HAL_NVIC_SystemReset();
}

static inline uint32_t sys_get_vector_table_address(void)
{
    return SCB->VTOR;
}

/**
 * @brief Calculate the CRC-16 checksum of a data buffer.
 *
 * Uses the CRC-16/MODBUS algorithm with an initial value of 0xFFFF
 * and polynomial 0xA001.
 *
 * @param[in] pData Data buffer.
 * @param[in] u16Size Data size in bytes.
 *
 * @return Calculated 16-bit CRC value.
 */
uint16_t sys_crc16(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Enter STOP2 low-power mode.
 *
 * Suspends the HAL tick, enters STOP2 mode using WFI, restores the system
 * clock configuration after wake-up, and resumes the HAL tick.
 */
void sys_sleep(void);

/**
 * @brief Generate a 32-bit random number.
 *
 * Uses the hardware RNG when available, falling back to the standard library
 * pseudo-random number generator if hardware generation fails.
 *
 * @return Generated 32-bit random number.
 */
uint32_t sys_get_random_number(void);