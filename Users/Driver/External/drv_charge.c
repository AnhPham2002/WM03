#include "drv_charge.h"

static bool bChargeIntFlag = false;

void drv_charge_init(void)
{
    drv_charge_enable();
}

void drv_charge_enable(void)
{
    drv_gpio_write(CHG_EN_PIN, 0);
}

void drv_charge_disable(void)
{
    drv_gpio_write(CHG_EN_PIN, 1);
}

bool drv_charge_write(uint8_t u8RegAddr, uint8_t u8Data)
{
    uint8_t au8TxData[] = {u8RegAddr, u8Data};
    return drv_i2c_send(CHG_TARGET_ADDR, au8TxData, sizeof(au8TxData));
}

bool drv_charge_read(uint8_t u8RegAddr, uint8_t *u8Data)
{
    if (!drv_i2c_send(CHG_TARGET_ADDR, &u8RegAddr, 1))
    {
        return false;
    }
    return drv_i2c_receive(CHG_TARGET_ADDR, u8Data, 1);
}

void drv_charge_interrupt(void)
{
    bChargeIntFlag = true;
}

bool drv_charge_fault_detected(void)
{
    if (bChargeIntFlag)
    {
        bChargeIntFlag = false;
        return true;
    }
    return false;
}