#include "drv_wkup_pin.h"

bool drv_wkup_is_triggered(void)
{
    return drv_gpio_irq_pending();
}