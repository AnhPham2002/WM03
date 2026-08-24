#pragma once

#include "drv_gpio.h"
#include "drv_timer.h"
#include "sys_common.h"

#include <stdbool.h>
#include <stdint.h>

#define PULSE_READ_PERIOD 50 // ms
#define PULSE_FREQUENCY_TIMEOUT 10000 // ms

#define PULSE1_IN_PIN GPIOA, GPIO_PIN_8
#define PULSE2_IN_PIN GPIOA, GPIO_PIN_9
#define PULSE3_IN_PIN GPIOA, GPIO_PIN_10
#define PULSE4_IN_PIN GPIOA, GPIO_PIN_11
#define PULSE_EN_PIN GPIOC, GPIO_PIN_9

typedef enum
{
    PULSE1_READ = 0,
    PULSE2_READ,
    PULSE3_READ,
    PULSE4_READ
} Pulse_Read_Select_t;

typedef enum
{
    EDGE_NONE = 0,
    EDGE_RISING,
    EDGE_FALLING
} Edge_Status_t;

typedef struct
{
    uint64_t u64Pulse1Count;
    uint64_t u64Pulse2Count;
    uint64_t u64Pulse3Count;
    uint64_t u64Pulse4Count;
} Pulse_Count_t;

typedef struct
{
    float fPulse1Frequency;
    float fPulse2Frequency;
    float fPulse3Frequency;
    float fPulse4Frequency;
} Pulse_Frequency_t;

/**
 * @brief Initialize pulse measurement.
 *
 * @param[in] pCount Initial pulse count.
 */
void drv_pulse_init(Pulse_Count_t *pCount);

/**
 * @brief Handle pulse measurement interrupt.
 *
 * @param[in] bReadable Pulse input readability status.
 */
void drv_pulse_interrupt_handler(bool bReadable);

/**
 * @brief Get pulse count and frequency.
 *
 * @param[out] pCount     Pulse count.
 * @param[out] pFrequency Pulse frequency.
 */
void drv_pulse_get_data(Pulse_Count_t *pCount, Pulse_Frequency_t *pFrequency);