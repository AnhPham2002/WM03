#pragma once

#include "setting.h"

#define WDI_PIN GPIOC, GPIO_PIN_13

void drv_wdt_init(void);
void drv_wdr_restart(void);