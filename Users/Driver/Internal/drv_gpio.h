#pragma once

#include "gpio.h"
#include "stm32l431xx.h"
#include "stm32l4xx_hal_gpio.h"
#include <stdint.h>

void drv_gpio_write(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
GPIO_PinState drv_gpio_read(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void drv_gpio_toggle(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void drv_gpio_deinit(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);