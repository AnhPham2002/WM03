#include "drv_pulse.h"

static volatile bool bPreviousPulse1Status;
static volatile bool bPreviousPulse2Status;
static volatile bool bPreviousPulse3Status;
static volatile bool bPreviousPulse4Status;

static Pulse_Config_t sPulseConfig[MAX_PULSE_GATE_COUNT];
static volatile double dPulseFrequency[MAX_PULSE_GATE_COUNT];

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
 * @brief Update pulse count based on detected pulse edges.
 *
 * Detects pulse edges for single pulse inputs and updates the forward
 * pulse count and CRC when a pulse is detected.
 */
static void drv_pulse_update_data(void);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void drv_pulse_init(const Pulse_Count_t *pCount)
{
    uint64_t u64Magic = *(volatile uint64_t *)RAM_NOINIT_MAGIC_ADDRESS;
    uint16_t u16Crc = *pPulseCountCrc;

    if ((u64Magic != RAM_NOINIT_MAGIC_NUMBER) || (sys_crc16((uint8_t *)pPulseCount, sizeof(Pulse_Count_t) * MAX_PULSE_GATE_COUNT) != u16Crc))
    {
        memcpy((void *)pPulseCount, pCount, sizeof(Pulse_Count_t) * MAX_PULSE_GATE_COUNT);
        *pPulseCountCrc = sys_crc16((uint8_t *)pPulseCount, sizeof(Pulse_Count_t) * MAX_PULSE_GATE_COUNT);
    }

    drv_pulse_read_enable();
    sys_delay_ms(1); // Delay for capacitor charge
    bPreviousPulse1Status = drv_pulse1_read();
    bPreviousPulse2Status = drv_pulse2_read();
    bPreviousPulse3Status = drv_pulse3_read();
    bPreviousPulse4Status = drv_pulse4_read();
    drv_pulse_read_disable();

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

bool drv_pulse_set_count(uint8_t u8Index, const Pulse_Count_t *pCount)
{
    if ((u8Index >= MAX_PULSE_GATE_COUNT) || (pCount == NULL))
    {
        return false;
    }

    pPulseCount[u8Index] = *pCount;
    *pPulseCountCrc = sys_crc16((uint8_t *)pPulseCount, sizeof(Pulse_Count_t) * MAX_PULSE_GATE_COUNT);

    return true;
}

bool drv_pulse_set_config(uint8_t u8Index, const Pulse_Config_t *pConfig)
{
    if ((u8Index >= MAX_PULSE_GATE_COUNT) || (pConfig == NULL))
    {
        return false;
    }

    sPulseConfig[u8Index] = *pConfig;

    return true;
}

bool drv_pulse_get_data(uint8_t u8Index, Pulse_Data_t *pData)
{
    if ((u8Index >= MAX_PULSE_GATE_COUNT) || (pData == NULL))
    {
        return false;
    }

    pData->u64ForwardPulseCount = pPulseCount[u8Index].u64ForwardPulseCount;
    pData->u64ReversePulseCount = pPulseCount[u8Index].u64ReversePulseCount;
    pData->dPulseFrequency = dPulseFrequency[u8Index];

    return true;
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
    bool bCountChange = false;

    for (uint8_t i = 0; i < MAX_PULSE_GATE_COUNT; i++)
    {
        if ((sPulseConfig[i].u8Pin1Select == 0) || (sPulseConfig[i].u8PulseType == 0) || (sPulseConfig[i].u8EdgeType == 0))
        {
            continue;
        }

        if (sPulseConfig[i].u8PulseType == PULSE_TYPE_SINGLE)
        {
            eEdgeDetect = drv_pulse_edge_detect(sPulseConfig[i].u8Pin1Select);
            if (eEdgeDetect == sPulseConfig[i].u8EdgeType)
            {
                pPulseCount[i].u64ForwardPulseCount++;
                bCountChange = true;
            }
        }
    }

    if (bCountChange)
    {
        *pPulseCountCrc = sys_crc16((uint8_t *)pPulseCount, sizeof(Pulse_Count_t) * MAX_PULSE_GATE_COUNT);
    }
}