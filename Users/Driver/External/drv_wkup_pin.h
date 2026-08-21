#pragma once

#include "drv_gpio.h"

/**
 * @brief Check whether the wake-up event is triggered.
 *
 * @return true if the wake-up event is triggered, otherwise false.
 */
bool drv_wkup_is_triggered(void);