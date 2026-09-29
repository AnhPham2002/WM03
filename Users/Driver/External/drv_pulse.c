#include "drv_pulse.h"

#define RAM_NOINIT_PULSE_COUNT_MAGIC_NUMBER 0x12345678

static volatile bool bPreviousPulse1Status;
static volatile bool bPreviousPulse2Status;
static volatile bool bPreviousPulse3Status;
static volatile bool bPreviousPulse4Status;

static Pulse_Config_t sPulseConfig[MAX_PULSE_GATE_COUNT];
static volatile double dPulseFrequency[MAX_PULSE_GATE_COUNT];

static volatile uint32_t u32PulseCountMagic RAM_NOINIT;
static volatile Pulse_Count_t sPulseCount[MAX_PULSE_GATE_COUNT] RAM_NOINIT;
static volatile uint16_t u16PulseCountCrc RAM_NOINIT;

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Enable pulse input 1 reading.
 */
static inline void drv_pulse1_read_enable(void);

/**
 * @brief Disable pulse input 1 reading.
 */
static inline void drv_pulse1_read_disable(void);

/**
 * @brief Enable pulse input 2 reading.
 */
static inline void drv_pulse2_read_enable(void);

/**
 * @brief Disable pulse input 2 reading.
 */
static inline void drv_pulse2_read_disable(void);

/**
 * @brief Enable pulse input 3 reading.
 */
static inline void drv_pulse3_read_enable(void);

/**
 * @brief Disable pulse input 3 reading.
 */
static inline void drv_pulse3_read_disable(void);

/**
 * @brief Enable pulse input 4 reading.
 */
static inline void drv_pulse4_read_enable(void);

/**
 * @brief Disable pulse input 4 reading.
 */
static inline void drv_pulse4_read_disable(void);

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
static Edge_Status_t drv_pulse_edge_detect(Pulse_Input_t ePulseIn);

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

void drv_pulse_init(void)
{
    drv_pulse1_read_enable();
    drv_pulse2_read_enable();
    drv_pulse3_read_enable();
    drv_pulse4_read_enable();
    sys_delay_ms(1); // Delay for capacitor charge
    bPreviousPulse1Status = drv_pulse1_read();
    bPreviousPulse2Status = drv_pulse2_read();
    bPreviousPulse3Status = drv_pulse3_read();
    bPreviousPulse4Status = drv_pulse4_read();
    drv_pulse1_read_disable();
    drv_pulse2_read_disable();
    drv_pulse3_read_disable();
    drv_pulse4_read_disable();

    drv_timer1_low_power_init(PULSE_READ_PERIOD);
}

void drv_pulse_count_init(const Pulse_Count_t *pCount)
{
    if ((u32PulseCountMagic != RAM_NOINIT_PULSE_COUNT_MAGIC_NUMBER) || (sys_crc16((uint8_t *)&sPulseCount, sizeof(sPulseCount)) != u16PulseCountCrc))
    {
        __disable_irq();
        memcpy((void *)&sPulseCount, pCount, sizeof(sPulseCount));
        u16PulseCountCrc = sys_crc16((uint8_t *)&sPulseCount, sizeof(sPulseCount));
        __enable_irq();
        u32PulseCountMagic = RAM_NOINIT_PULSE_COUNT_MAGIC_NUMBER;
    }
}

void drv_pulse_interrupt_handler(bool bReadable)
{
    if (!bReadable)
    {
        drv_pulse1_read_enable();
        drv_pulse2_read_enable();
        drv_pulse3_read_enable();
        drv_pulse4_read_enable();
    }
    else
    {
        drv_pulse_update_data();
        drv_pulse1_read_disable();
        drv_pulse2_read_disable();
        drv_pulse3_read_disable();
        drv_pulse4_read_disable();
    }
}

bool drv_pulse_set_count(uint8_t u8Index, const Pulse_Count_t *pCount)
{
    if ((u8Index >= MAX_PULSE_GATE_COUNT) || (pCount == NULL))
    {
        return false;
    }

    __disable_irq();
    memcpy((void *)&sPulseCount[u8Index], pCount, sizeof(Pulse_Count_t));
    u16PulseCountCrc = sys_crc16((uint8_t *)&sPulseCount, sizeof(sPulseCount));
    __enable_irq();

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

    __disable_irq();
    pData->u64ForwardPulseCount = sPulseCount[u8Index].u64ForwardPulseCount;
    pData->u64ReversePulseCount = sPulseCount[u8Index].u64ReversePulseCount;
    __enable_irq();
    pData->dPulseFrequency = dPulseFrequency[u8Index];

    return true;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static inline void drv_pulse1_read_enable(void)
{
    drv_gpio_write(PULSE1_EN_PIN, 0);
}

static inline void drv_pulse1_read_disable(void)
{
    drv_gpio_write(PULSE1_EN_PIN, 1);
}

static inline void drv_pulse2_read_enable(void)
{
    drv_gpio_write(PULSE2_EN_PIN, 0);
}

static inline void drv_pulse2_read_disable(void)
{
    drv_gpio_write(PULSE2_EN_PIN, 1);
}

static inline void drv_pulse3_read_enable(void)
{
    drv_gpio_write(PULSE3_EN_PIN, 0);
}

static inline void drv_pulse3_read_disable(void)
{
    drv_gpio_write(PULSE3_EN_PIN, 1);
}

static inline void drv_pulse4_read_enable(void)
{
    drv_gpio_write(PULSE4_EN_PIN, 0);
}

static inline void drv_pulse4_read_disable(void)
{
    drv_gpio_write(PULSE4_EN_PIN, 1);
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

static Edge_Status_t drv_pulse_edge_detect(Pulse_Input_t ePulseIn)
{
    bool bCurrentPulseStatus;
    volatile bool *pPreviousPulseStatus;

    switch (ePulseIn)
    {
    case PULSE_INPUT_1:
        pPreviousPulseStatus = &bPreviousPulse1Status;
        bCurrentPulseStatus = drv_pulse1_read();
        break;

    case PULSE_INPUT_2:
        pPreviousPulseStatus = &bPreviousPulse2Status;
        bCurrentPulseStatus = drv_pulse2_read();
        break;

    case PULSE_INPUT_3:
        pPreviousPulseStatus = &bPreviousPulse3Status;
        bCurrentPulseStatus = drv_pulse3_read();
        break;

    case PULSE_INPUT_4:
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
                sPulseCount[i].u64ForwardPulseCount++;
                bCountChange = true;
            }
        }
    }

    if (bCountChange)
    {
        u16PulseCountCrc = sys_crc16((uint8_t *)&sPulseCount, sizeof(sPulseCount));
    }
}