#include "sv_charge.h"

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Enable charger configuration.
 *
 * @return true if the charger configuration is enabled successfully, otherwise false.
 */
static bool sv_charge_config_enable(void);

/**
 * @brief Disable charger configuration.
 *
 * @return true if the charger configuration is disabled successfully, otherwise false.
 */
static bool sv_charge_config_disable(void);

/**
 * @brief Start charger ADC conversion.
 *
 * @return true if the conversion is started successfully, otherwise false.
 */
static bool sv_charge_start_adc_conversion(void);

/**
 * @brief Get VBUS status.
 *
 * @param[out] u8Status VBUS status.
 *
 * @return true if the status is read successfully, otherwise false.
 */
static bool sv_charge_get_vbus_status(uint8_t *u8Status);

/**
 * @brief Get charging status.
 *
 * @param[out] u8Status Charging status.
 *
 * @return true if the status is read successfully, otherwise false.
 */
static bool sv_charge_get_charging_status(uint8_t *u8Status);

/**
 * @brief Get battery voltage.
 *
 * @param[out] u16VoltageMv Battery voltage in mV.
 *
 * @return true if the voltage is read successfully, otherwise false.
 */
static bool sv_charge_get_battery_voltage(uint16_t *u16VoltageMv);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void sv_charge_init(uint16_t u16IinLimMa, uint16_t u16VoltageMv, uint16_t u16Current, bool LedStatus)
{
    drv_charge_init();

    sv_charge_config_enable();
    sv_charge_set_input_current_limit(u16IinLimMa);
    sv_charge_set_charge_voltage(u16VoltageMv);
    sv_charge_set_fast_charge_current(u16Current);
    if (LedStatus)
    {
        sv_charge_turn_on_led_status();
    }
    else
    {
        sv_charge_turn_off_led_status();
    }

    sv_charge_wd_rst();
}

bool sv_charge_interrupt_detected(void)
{
    return drv_charge_interrupt_detected();
}

bool sv_charge_set_input_current_limit(uint16_t u16CurrentMa)
{
    if (u16CurrentMa < CHG_IINLIM_MIN_MA)
    {
        u16CurrentMa = CHG_IINLIM_MIN_MA;
    }
    else if (u16CurrentMa > CHG_IINLIM_MAX_MA)
    {
        u16CurrentMa = CHG_IINLIM_MAX_MA;
    }

    uint8_t u8IinLimBit = (u16CurrentMa - CHG_IINLIM_OFFSET_MA) / CHG_IINLIM_STEP_MA;
    uint8_t u8Reg00Value;
    if (!drv_charge_read(CHG_REG00, &u8Reg00Value))
    {
        return false;
    }

    u8Reg00Value &= ~CHG_REG00_IINLIM_MASK;
    u8Reg00Value |= (u8IinLimBit << CHG_REG00_IINLIM_POS) & CHG_REG00_IINLIM_MASK;
    return drv_charge_write(CHG_REG00, u8Reg00Value);
}

bool sv_charge_wd_rst(void)
{
    uint8_t u8Reg03Value;
    if (!drv_charge_read(CHG_REG03, &u8Reg03Value))
    {
        return false;
    }

    u8Reg03Value = (u8Reg03Value & ~CHG_REG03_WD_RST_MASK) | (CHG_REG03_WD_RST_RESET << CHG_REG03_WD_RST_POS);
    return drv_charge_write(CHG_REG03, u8Reg03Value);
}

bool sv_charge_set_fast_charge_current(uint16_t u16CurrentMa)
{
    if (u16CurrentMa < CHG_ICHG_MIN_MA)
    {
        u16CurrentMa = CHG_ICHG_MIN_MA;
    }
    else if (u16CurrentMa > CHG_ICHG_MAX_MA)
    {
        u16CurrentMa = CHG_ICHG_MAX_MA;
    }

    uint8_t u8IchgBit = (u16CurrentMa - CHG_ICHG_OFFSET_MA) / CHG_ICHG_STEP_MA;
    uint8_t u8Reg04Value;
    if (!drv_charge_read(CHG_REG04, &u8Reg04Value))
    {
        return false;
    }

    u8Reg04Value &= ~CHG_REG04_ICHG_MASK;
    u8Reg04Value |= (u8IchgBit << CHG_REG04_ICHG_POS) & CHG_REG04_ICHG_MASK;
    return drv_charge_write(CHG_REG04, u8Reg04Value);
}

bool sv_charge_set_charge_voltage(uint16_t u16VoltageMv)
{
    if (u16VoltageMv < CHG_VREG_MIN_MV)
    {
        u16VoltageMv = CHG_VREG_MIN_MV;
    }
    else if (u16VoltageMv > CHG_VREG_MAX_MV)
    {
        u16VoltageMv = CHG_VREG_MAX_MV;
    }

    uint8_t u8VregBit = (u16VoltageMv - CHG_VREG_OFFSET_MV) / CHG_VREG_STEP_MV;
    uint8_t u8Reg06Value;
    if (!drv_charge_read(CHG_REG06, &u8Reg06Value))
    {
        return false;
    }

    u8Reg06Value &= ~CHG_REG06_VREG_MASK;
    u8Reg06Value |= (u8VregBit << CHG_REG06_VREG_POS) & CHG_REG06_VREG_MASK;
    return drv_charge_write(CHG_REG06, u8Reg06Value);
}

bool sv_charge_turn_on_led_status(void)
{
    uint8_t u8Reg07Value;
    if (!drv_charge_read(CHG_REG07, &u8Reg07Value))
    {
        return false;
    }

    u8Reg07Value = (u8Reg07Value & ~CHG_REG07_STAT_DIS_MASK) | (CHG_REG07_STAT_EN << CHG_REG07_STAT_DIS_POS);
    return drv_charge_write(CHG_REG07, u8Reg07Value);
}

bool sv_charge_turn_off_led_status(void)
{
    uint8_t u8Reg07Value;
    if (!drv_charge_read(CHG_REG07, &u8Reg07Value))
    {
        return false;
    }

    u8Reg07Value = (u8Reg07Value & ~CHG_REG07_STAT_DIS_MASK) | (CHG_REG07_STAT_DIS << CHG_REG07_STAT_DIS_POS);
    return drv_charge_write(CHG_REG07, u8Reg07Value);
}

bool sv_charge_disconnect_battery(void)
{
    uint8_t u8Reg09Value;
    if (!drv_charge_read(CHG_REG09, &u8Reg09Value))
    {
        return false;
    }

    u8Reg09Value = (u8Reg09Value & ~CHG_REG09_BATFET_DIS_MASK) | (CHG_REG09_BATFET_OFF << CHG_REG09_BATFET_DIS_POS);
    return drv_charge_write(CHG_REG09, u8Reg09Value);
}

bool sv_charge_get_fault(uint8_t *u8Fault)
{
    return drv_charge_read(CHG_REG0C, u8Fault);
}

void sv_charge_get_power_info(bool *bVbusStatus, uint8_t *u8ChargingStatus, uint16_t *u16BatteryVoltageMv)
{
    uint8_t u8VbusStatus;
    sv_charge_get_vbus_status(&u8VbusStatus);

    if (u8VbusStatus == CHG_VBUS_STAT_NO_INPUT)
    {
        *bVbusStatus = false;
        *u8ChargingStatus = CHG_CHRG_STAT_NOT_CHARGING;
        sv_charge_start_adc_conversion();
        sys_delay_ms(50); // Delay for conversion
        sv_charge_get_battery_voltage(u16BatteryVoltageMv);
    }
    else
    {
        *bVbusStatus = true;
        sv_charge_config_disable();
        sys_delay_ms(50); // Delay for conversion
        sv_charge_start_adc_conversion();
        sys_delay_ms(50); // Delay for conversion
        sv_charge_get_battery_voltage(u16BatteryVoltageMv);
        sv_charge_config_enable();
        if (*u16BatteryVoltageMv == CHG_BATV_MIN_MV)
        {
            *u8ChargingStatus = CHG_CHRG_STAT_NOT_CHARGING;
            *u16BatteryVoltageMv = 0;
        }
        else
        {
            sv_charge_get_charging_status(u8ChargingStatus);
        }
    }
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static bool sv_charge_config_enable(void)
{
    uint8_t u8Reg03Value;
    if (!drv_charge_read(CHG_REG03, &u8Reg03Value))
    {
        return false;
    }

    u8Reg03Value = (u8Reg03Value & ~CHG_REG03_CHG_CONFIG_MASK) | (CHG_REG03_CHG_EN << CHG_REG03_CHG_CONFIG_POS);
    return drv_charge_write(CHG_REG03, u8Reg03Value);
}

static bool sv_charge_config_disable(void)
{
    uint8_t u8Reg03Value;
    if (!drv_charge_read(CHG_REG03, &u8Reg03Value))
    {
        return false;
    }

    u8Reg03Value = (u8Reg03Value & ~CHG_REG03_CHG_CONFIG_MASK) | (CHG_REG03_CHG_DIS << CHG_REG03_CHG_CONFIG_POS);
    return drv_charge_write(CHG_REG03, u8Reg03Value);
}

static bool sv_charge_start_adc_conversion(void)
{
    uint8_t u8Reg02Value;
    if (!drv_charge_read(CHG_REG02, &u8Reg02Value))
    {
        return false;
    }

    u8Reg02Value = (u8Reg02Value & ~CHG_REG02_CONV_START_MASK) | (CHG_REG02_CONV_START_ACTIVE << CHG_REG02_CONV_START_POS);
    return drv_charge_write(CHG_REG02, u8Reg02Value);
}

static bool sv_charge_get_vbus_status(uint8_t *u8Status)
{
    uint8_t u8Reg0bValue;
    *u8Status = 0;
    if (!drv_charge_read(CHG_REG0B, &u8Reg0bValue))
    {
        return false;
    }

    *u8Status = (u8Reg0bValue & CHG_REG0B_VBUS_STAT_MASK) >> CHG_REG0B_VBUS_STAT_POS;
    return true;
}

static bool sv_charge_get_charging_status(uint8_t *u8Status)
{
    uint8_t u8Reg0bValue;
    *u8Status = 0;
    if (!drv_charge_read(CHG_REG0B, &u8Reg0bValue))
    {
        return false;
    }

    *u8Status = (u8Reg0bValue & CHG_REG0B_CHRG_STAT_MASK) >> CHG_REG0B_CHRG_STAT_POS;
    return true;
}

static bool sv_charge_get_battery_voltage(uint16_t *u16VoltageMv)
{
    uint8_t u8Reg0eValue;
    *u16VoltageMv = 0;
    if (!drv_charge_read(CHG_REG0E, &u8Reg0eValue))
    {
        return false;
    }

    uint8_t u8BatvBit = (u8Reg0eValue & CHG_REG0E_BATV_MASK) >> CHG_REG0E_BATV_POS;
    *u16VoltageMv = (u8BatvBit * CHG_BATV_STEP_MV) + CHG_BATV_OFFSET_MV;
    return true;
}