#include "app_main.h"
#include "iwdg.h"

void wdg_rst(void)
{
    HAL_IWDG_Refresh(&hiwdg);
}