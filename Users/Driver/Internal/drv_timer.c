#include "drv_timer.h"
#include "lptim.h"

void drv_timer1_low_power_init(uint32_t u32Period)
{
    HAL_LPTIM_PWM_Start_IT(&hlptim1, u32Period * LPTIM1_CLOCK_SOURCE, (u32Period - 1) * LPTIM1_CLOCK_SOURCE);
}

void HAL_LPTIM_CompareMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
}

void HAL_LPTIM_AutoReloadMatchCallback(LPTIM_HandleTypeDef *hlptim)
{
}