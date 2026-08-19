#include "sys_debug.h"

void sys_log(const uint8_t *pData, uint16_t u16Size)
{
#ifdef USE_DEBUG
    drv_uart2_send(pData, u16Size);
#endif
}

bool sys_console(uint8_t *pData, uint16_t *u16Size)
{
#ifdef USE_DEBUG
    return drv_uart2_receive(pData, u16Size);
#endif
}