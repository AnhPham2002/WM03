#include "drv_pulse.h"

static volatile bool bPreviousPulse1Status;
static volatile bool bPreviousPulse2Status;
static volatile bool bPreviousPulse3Status;
static volatile bool bPreviousPulse4Status;

static volatile uint32_t u32PreviousEdgePulse1DetectTime;
static volatile uint32_t u32PreviousEdgePulse2DetectTime;
static volatile uint32_t u32PreviousEdgePulse3DetectTime;
static volatile uint32_t u32PreviousEdgePulse4DetectTime;

static volatile Pulse_Frequency_t sPulseFrequency;

static volatile Pulse_Count_t *const pPulseCount = (volatile Pulse_Count_t *)RAM_NOINIT_PULSE_COUNT_ADDRESS;
static volatile uint16_t *const pPulseCountCrc = (volatile uint16_t *)RAM_NOINIT_PULSE_COUNT_CRC_ADDRESS;

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Enable pulse reading.
 */
static inline void drv_pulse_read_enable(void);

/**
 * @brief Disable pulse reading.
 */
static inline void drv_pulse_read_disable(void);

/**
 * @brief Read pulse input 1.
 *
 * @return true if pulse is active, otherwise false.
 */
static inline bool drv_pulse1_read(void);

/**
 * @brief Read pulse input 2.
 *
 * @return true if pulse is active, otherwise false.
 */
static inline bool drv_pulse2_read(void);

/**
 * @brief Read pulse input 3.
 *
 * @return true if pulse is active, otherwise false.
 */
static inline bool drv_pulse3_read(void);

/**
 * @brief Read pulse input 4.
 *
 * @return true if pulse is active, otherwise false.
 */
static inline bool drv_pulse4_read(void);

/**
 * @brief Detect pulse input edge.
 *
 * @param[in] ePulseIn Pulse input to check.
 *
 * @return Detected edge status.
 */
static Edge_Status_t drv_pulse_edge_detect(Pulse_Read_Select_t ePulseIn);

/**
 * @brief  Update pulse count and frequency data for all pulse inputs.
 *
 * This function detects pulse edges, updates the accumulated pulse count,
 * and calculates the pulse frequency based on the time interval between
 * consecutive detected edges. If no edge is detected within the configured
 * timeout period, the corresponding pulse frequency is set to zero.
 *
 * @note   The pulse frequency calculation depends on the system time
 *         resolution and the edge detection period.
 *
 * @return None.
 */
static void drv_pulse_update_data(void);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void drv_pulse_init(Pulse_Count_t *pCount)
{
    uint64_t u64Magic = *(volatile uint64_t *)RAM_NOINIT_MAGIC_ADDRESS;
    uint16_t u16Crc = *(volatile uint16_t *)RAM_NOINIT_PULSE_COUNT_CRC_ADDRESS;
    if ((u64Magic != RAM_NOINIT_MAGIC_NUMBER) || (sys_crc_16((uint8_t *)pPulseCount, sizeof(Pulse_Count_t)) != u16Crc))
    {
        *pPulseCount = *pCount;
        *pPulseCountCrc = sys_crc_16((uint8_t *)pPulseCount, sizeof(Pulse_Count_t));
    }

    drv_pulse_read_enable();
    sys_delay_ms(1); // Delay for capacitor charge
    bPreviousPulse1Status = drv_pulse1_read();
    bPreviousPulse2Status = drv_pulse2_read();
    bPreviousPulse3Status = drv_pulse3_read();
    bPreviousPulse4Status = drv_pulse4_read();
    drv_pulse_read_disable();

    u32PreviousEdgePulse1DetectTime = sys_time_ms();
    u32PreviousEdgePulse2DetectTime = u32PreviousEdgePulse1DetectTime;
    u32PreviousEdgePulse3DetectTime = u32PreviousEdgePulse2DetectTime;
    u32PreviousEdgePulse4DetectTime = u32PreviousEdgePulse3DetectTime;

    drv_timer1_low_power_init(PULSE_READ_PERIOD);
}

void drv_pulse_interrupt_handler(bool bReadable)
{
    if (!bReadable)
    {
        drv_pulse_read_enable();
    }
    else
    {
        drv_pulse_update_data();
        drv_pulse_read_disable();
    }
}

void drv_pulse_get_data(Pulse_Count_t *pCount, Pulse_Frequency_t *pFrequency)
{
    *pCount = *pPulseCount;
    *pFrequency = sPulseFrequency;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static inline void drv_pulse_read_enable(void)
{
    drv_gpio_write(PULSE_EN_PIN, 1);
}

static inline void drv_pulse_read_disable(void)
{
    drv_gpio_write(PULSE_EN_PIN, 0);
}

static inline bool drv_pulse1_read(void)
{
    return drv_gpio_read(PULSE1_IN_PIN);
}

static inline bool drv_pulse2_read(void)
{
    return drv_gpio_read(PULSE2_IN_PIN);
}

static inline bool drv_pulse3_read(void)
{
    return drv_gpio_read(PULSE3_IN_PIN);
}

static inline bool drv_pulse4_read(void)
{
    return drv_gpio_read(PULSE4_IN_PIN);
}

static Edge_Status_t drv_pulse_edge_detect(Pulse_Read_Select_t ePulseIn)
{
    bool bCurrentPulseStatus;
    volatile bool *pPreviousPulseStatus;

    switch (ePulseIn)
    {
    case PULSE1_READ:
        pPreviousPulseStatus = &bPreviousPulse1Status;
        bCurrentPulseStatus = drv_pulse1_read();
        break;

    case PULSE2_READ:
        pPreviousPulseStatus = &bPreviousPulse2Status;
        bCurrentPulseStatus = drv_pulse2_read();
        break;

    case PULSE3_READ:
        pPreviousPulseStatus = &bPreviousPulse3Status;
        bCurrentPulseStatus = drv_pulse3_read();
        break;

    case PULSE4_READ:
        pPreviousPulseStatus = &bPreviousPulse4Status;
        bCurrentPulseStatus = drv_pulse4_read();
        break;

    default:
        return EDGE_NONE;
    }

    if (*pPreviousPulseStatus == bCurrentPulseStatus)
    {
        return EDGE_NONE;
    }

    *pPreviousPulseStatus = bCurrentPulseStatus;

    return bCurrentPulseStatus ? EDGE_RISING : EDGE_FALLING;
}

static void drv_pulse_update_data(void)
{
    Edge_Status_t eEdgeDetect;
    uint32_t u32CurrentEdgePulseDetectTime = sys_time_ms();

    eEdgeDetect = drv_pulse_edge_detect(PULSE1_READ);
    if (eEdgeDetect == EDGE_NONE)
    {
        if (u32CurrentEdgePulseDetectTime - u32PreviousEdgePulse1DetectTime >= PULSE_FREQUENCY_TIMEOUT)
        {
            sPulseFrequency.fPulse1Frequency = 0.0f;
        }
    }
    else
    {
        sPulseFrequency.fPulse1Frequency = 500.0f / (u32CurrentEdgePulseDetectTime - u32PreviousEdgePulse1DetectTime);
        u32PreviousEdgePulse1DetectTime = u32CurrentEdgePulseDetectTime;

        if (eEdgeDetect == EDGE_FALLING)
        {
            pPulseCount->u64Pulse1Count++;
        }
    }

    eEdgeDetect = drv_pulse_edge_detect(PULSE2_READ);
    if (eEdgeDetect == EDGE_NONE)
    {
        if (u32CurrentEdgePulseDetectTime - u32PreviousEdgePulse2DetectTime >= PULSE_FREQUENCY_TIMEOUT)
        {
            sPulseFrequency.fPulse2Frequency = 0.0f;
        }
    }
    else
    {
        sPulseFrequency.fPulse2Frequency = 500.0f / (u32CurrentEdgePulseDetectTime - u32PreviousEdgePulse2DetectTime);
        u32PreviousEdgePulse2DetectTime = u32CurrentEdgePulseDetectTime;

        if (eEdgeDetect == EDGE_FALLING)
        {
            pPulseCount->u64Pulse2Count++;
        }
    }

    eEdgeDetect = drv_pulse_edge_detect(PULSE3_READ);
    if (eEdgeDetect == EDGE_NONE)
    {
        if (u32CurrentEdgePulseDetectTime - u32PreviousEdgePulse3DetectTime >= PULSE_FREQUENCY_TIMEOUT)
        {
            sPulseFrequency.fPulse3Frequency = 0.0f;
        }
    }
    else
    {
        sPulseFrequency.fPulse3Frequency = 500.0f / (u32CurrentEdgePulseDetectTime - u32PreviousEdgePulse3DetectTime);
        u32PreviousEdgePulse3DetectTime = u32CurrentEdgePulseDetectTime;

        if (eEdgeDetect == EDGE_FALLING)
        {
            pPulseCount->u64Pulse3Count++;
        }
    }

    eEdgeDetect = drv_pulse_edge_detect(PULSE4_READ);
    if (eEdgeDetect == EDGE_NONE)
    {
        if (u32CurrentEdgePulseDetectTime - u32PreviousEdgePulse4DetectTime >= PULSE_FREQUENCY_TIMEOUT)
        {
            sPulseFrequency.fPulse4Frequency = 0.0f;
        }
    }
    else
    {
        sPulseFrequency.fPulse4Frequency = 500.0f / (u32CurrentEdgePulseDetectTime - u32PreviousEdgePulse4DetectTime);
        u32PreviousEdgePulse4DetectTime = u32CurrentEdgePulseDetectTime;

        if (eEdgeDetect == EDGE_FALLING)
        {
            pPulseCount->u64Pulse4Count++;
        }
    }

    *pPulseCountCrc = sys_crc_16((uint8_t *)pPulseCount, sizeof(Pulse_Count_t));
}