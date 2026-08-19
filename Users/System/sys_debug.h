#pragma once

#include "setting.h"
#include "drv_uart.h"

void sys_debug_init(void);
void sys_log(const uint8_t *pData, uint16_t u16Size);
bool sys_console(uint8_t *pData, uint16_t *u16Size);