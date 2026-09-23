#include "sv_485_communication.h"
#include "drv_led.h"

static bool bRs485Power = false;
static bool bCommunicateDetect = false;
static uint32_t bRs485CommunicateTime;

void sv_485_communication_init(void)
{
    drv_rs485_set_serial_config(RS485_COMMUNICATION_BAUDRATE, RS485_COMMUNICATION_SERIAL_CONFIG);
}

void sv_485_communication_power_process(void)
{
    if (drv_wkup_pin_falling())
    {
        drv_led_on();
        drv_rs485_pwr_on();
        bRs485Power = true;
        bRs485CommunicateTime = sys_time_ms();
    }

    if (bRs485Power)
    {
        drv_rs485_pwr_on();

        if (!bCommunicateDetect && drv_wkup_pin_rising())
        {
            bCommunicateDetect = false;
            drv_led_off();
            drv_rs485_pwr_off();
            bRs485Power = false;
        }

        if (sys_time_ms() - bRs485CommunicateTime >= RS485_COMMUNICATION_TIMEOUT)
        {
            bCommunicateDetect = false;
            drv_led_off();
            drv_rs485_pwr_off();
            bRs485Power = false;
        }
    }
}

bool sv_485_communication_send(const uint8_t *pData, uint16_t u16Size)
{
    bRs485CommunicateTime = sys_time_ms();
    return drv_rs485_send(pData, u16Size);
}

bool sv_485_communication_receive(uint8_t *pData, uint16_t *u16Size)
{
    if (drv_rs485_receive(pData, u16Size))
    {
        bCommunicateDetect = true;
        bRs485CommunicateTime = sys_time_ms();
        return true;
    }

    return false;
}