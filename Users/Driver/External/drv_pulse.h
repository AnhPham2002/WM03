#pragma once

#include "drv_gpio.h"
#include "drv_timer.h"
#include "sys_common.h"

#define MAX_PULSE_GATE_COUNT 4

#define PULSE_READ_PERIOD 50          // ms
#define PULSE_FREQUENCY_TIMEOUT 10000 // ms

#define PULSE1_IN_PIN GPIOA, GPIO_PIN_8
#define PULSE2_IN_PIN GPIOA, GPIO_PIN_9
#define PULSE3_IN_PIN GPIOA, GPIO_PIN_10
#define PULSE4_IN_PIN GPIOA, GPIO_PIN_11
#define PULSE_EN_PIN GPIOC, GPIO_PIN_9

/** @defgroup PULSE_TYPE Pulse Type
 *  @brief Pulse input type definitions.
 *  @{
 */
#define PULSE_TYPE_SINGLE 1U     // Single pulse input; only Pin1 is used.
#define PULSE_TYPE_ENCODER_AB 2U // Quadrature encoder input; Pin1 and Pin2 are used as A/B channels.
#define PULSE_TYPE_DIRECTION 3U  // Directional pulse input; Pin1 is the pulse input and Pin2 indicates direction.
/** @} */

typedef enum
{
    PULSE1_READ = 1,
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

typedef struct __attribute__((packed))
{
    uint8_t u8Pin1Select;
    uint8_t u8Pin2Select;
    uint8_t u8PulseType;
    uint8_t u8EdgeType;
} Pulse_Config_t;

typedef struct
{
    uint64_t u64ForwardPulseCount;
    uint64_t u64ReversePulseCount;
} Pulse_Count_t;

typedef struct
{
    uint64_t u64ForwardPulseCount;
    uint64_t u64ReversePulseCount;
    double dPulseFrequency;
} Pulse_Data_t;

/**
 * @brief Initialize pulse measurement.
 *
 * Restores pulse count from no-init RAM if the stored data is invalid,
 * initializes pulse input states, and starts the low-power timer.
 *
 * @param[in] pCount Default pulse count.
 */
void drv_pulse_init(const Pulse_Count_t *pCount);

/**
 * @brief Handle pulse measurement interrupt.
 *
 * @param[in] bReadable Pulse input readability status.
 */
void drv_pulse_interrupt_handler(bool bReadable);

/**
 * @brief Set pulse count for an input.
 *
 * @param[in] u8Index Index of the pulse input.
 * @param[in] pCount  Pulse count.
 *
 * @return true if the pulse count is set successfully, otherwise false.
 */
bool drv_pulse_set_count(uint8_t u8Index, const Pulse_Count_t *pCount);

/**
 * @brief Set pulse configuration for an input.
 *
 * @param[in] u8Index  Index of the pulse input.
 * @param[in] pConfig  Pulse configuration.
 *
 * @return true if the configuration is set successfully, otherwise false.
 */
bool drv_pulse_set_config(uint8_t u8Index, const Pulse_Config_t *pConfig);

/**
 * @brief Get pulse measurement data for an input.
 *
 * @param[in]  u8Index Index of the pulse input.
 * @param[out] pData  Pulse measurement data.
 *
 * @return true if the data is retrieved successfully, otherwise false.
 */
bool drv_pulse_get_data(uint8_t u8Index, Pulse_Data_t *pData);