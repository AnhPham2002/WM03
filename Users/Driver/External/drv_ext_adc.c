#include "drv_ext_adc.h"

#define EXT_ADC_CR_G EXT_ADC_CR_G_X4
#define EXT_ADC_CR_S EXT_ADC_CR_S_18BIT
#define EXT_ADC_CR_MODE EXT_ADC_CR_MODE_ONE_SHOT
#define EXT_ADC_CR_RDY EXT_ADC_CR_RDY_EN

#define EXT_ADC_CR ((EXT_ADC_CR_G << EXT_ADC_CR_G_POS) | (EXT_ADC_CR_S << EXT_ADC_CR_S_POS) | (EXT_ADC_CR_MODE << EXT_ADC_CR_MODE_POS) | (EXT_ADC_CR_RDY << EXT_ADC_CR_RDY_POS))

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Configure external ADC.
 *
 * @return true if configuration is successful, otherwise false.
 */
static inline bool drv_ext_adc_config(void);

/**
 * @brief Convert binary value to signed integer.
 *
 * @param[in] u32Bin Pointer to binary value.
 * @param[in] u8Bit  Number of valid bits.
 *
 * @return Signed integer value.
 */
static inline int32_t binary_to_signed(const uint32_t *u32Bin, uint8_t u8Bit);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void drv_ext_adc_pwr_on(void)
{
    drv_gpio_write(PWR_SS_PIN, 0);
    drv_gpio_write(PWR_EXT_ADC_PIN, 0);
}

void drv_ext_adc_pwr_off(void)
{
    drv_gpio_write(PWR_SS_PIN, 1);
    drv_gpio_write(PWR_EXT_ADC_PIN, 1);
}

void drv_ext_adc_init(void)
{
    drv_ext_adc_pwr_off();
    drv_ext_adc_disable_all_sensors();
    drv_i2c_init();
}

void drv_ext_adc_enable_sensor1(void)
{
    drv_gpio_write(EN_SS2_PIN, 0);
    drv_gpio_write(EN_SS1_PIN, 1);
}

void drv_ext_adc_enable_sensor2(void)
{
    drv_gpio_write(EN_SS1_PIN, 0);
    drv_gpio_write(EN_SS2_PIN, 1);
}

void drv_ext_adc_disable_all_sensors(void)
{
    drv_gpio_write(EN_SS1_PIN, 0);
    drv_gpio_write(EN_SS2_PIN, 0);
}

bool drv_ext_adc_read(float *fVal)
{
    uint8_t N; // Number of resolution bits (12/14/16/18)
    float LSB; // LSB weight corresponding to the selected resolution

#if EXT_ADC_CR_S == EXT_ADC_CR_S_12BIT
    N = 12;
    LSB = 1E-3; // 1 mV per LSB
#elif EXT_ADC_CR_S == EXT_ADC_CR_S_14BIT
    N = 14;
    LSB = 250E-6; // 250 µV per LSB
#elif EXT_ADC_CR_S == EXT_ADC_CR_S_16BIT
    N = 16;
    LSB = 62.5E-6; // 62.5 µV per LSB
#elif EXT_ADC_CR_S == EXT_ADC_CR_S_18BIT
    N = 18;
    LSB = 15.625E-6; // 15.625 µV per LSB
#else
    return false;
#endif

    // Select number of bytes to read:
    //   - 4 bytes for 18-bit mode
    //   - 3 bytes for all other resolutions
    uint8_t u8ReceiveDataSize = (N == 18) ? 4 : 3;

    uint8_t au8Data[4] = {0};   // Buffer for raw ADC data
    uint32_t u32Data = 0; // 32-bit container to assemble ADC value

    if (!drv_ext_adc_config())
    {
        return false;
    }

    bool bReadStatus = drv_i2c_receive(EXT_ADC_ADDR, au8Data, u8ReceiveDataSize);
    uint8_t u8CfgReg = au8Data[u8ReceiveDataSize - 1];

    if (!bReadStatus || ((u8CfgReg & EXT_ADC_CR_RDY_MASK) >> EXT_ADC_CR_RDY_POS == EXT_ADC_CR_RDY_EN)) // Retry
    {
        sys_delay_ms(EXT_ADC_CONVERSION_TIMEOUT);
        bReadStatus = drv_i2c_receive(EXT_ADC_ADDR, au8Data, u8ReceiveDataSize);
        u8CfgReg = au8Data[u8ReceiveDataSize - 1];

        // Return false if I2C failed or Ready bit isn't cleared or Configuration byte invalid
        if (!bReadStatus || ((u8CfgReg & EXT_ADC_CR_RDY_MASK) >> EXT_ADC_CR_RDY_POS == EXT_ADC_CR_RDY_EN) || ((u8CfgReg & 0x1F) != (EXT_ADC_CR & 0x1F)))
        {
            return false;
        }
    }

    for (uint8_t i = 0; i < u8ReceiveDataSize - 1; i++)
    {
        u32Data |= (uint32_t)(au8Data[i] << (8 * (u8ReceiveDataSize - i - 2))); // Shift each byte into correct big-endian position
    }

    // Convert raw ADC code into signed integer based on N bits,
	// multiply by LSB → convert to volts before PGA,
	// then divide by (1 << Gain) to compensate for PGA gain.
	*fVal = binary_to_signed(&u32Data, N) * LSB / (1 << EXT_ADC_CR_G);
    sys_log((const uint8_t *)fVal, 4);

    return true;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static inline bool drv_ext_adc_config(void)
{
    uint8_t u8CfgReg = EXT_ADC_CR;
    return drv_i2c_send((uint16_t)EXT_ADC_ADDR, &u8CfgReg, 1);
}

static inline int32_t binary_to_signed(const uint32_t *u32Bin, uint8_t u8Bit)
{
    uint32_t value = *u32Bin;
    value &= ((1U << u8Bit) - 1);
    if (value & (1U << (u8Bit - 1)))
    {
        value -= (1U << u8Bit);
    }
    return (int32_t)value;
}