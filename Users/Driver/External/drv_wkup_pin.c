#include "drv_wkup_pin.h"

bool drv_wkup_pin_rising(void)
{
    static bool bLastWkupPinStatus = true;

    bool bCurrentWkupPinStatus = drv_gpio_read(WKUP_PIN);

    if (bCurrentWkupPinStatus != bLastWkupPinStatus)
    {
        bLastWkupPinStatus = bCurrentWkupPinStatus;
        if (bCurrentWkupPinStatus)
        {
            return true;
        }
    }

    return false;
}

bool drv_wkup_pin_falling(void)
{
    static bool bLastWkupPinStatus = true;

    bool bCurrentWkupPinStatus = drv_gpio_read(WKUP_PIN);

    if (bCurrentWkupPinStatus != bLastWkupPinStatus)
    {
        bLastWkupPinStatus = bCurrentWkupPinStatus;
        if (!bCurrentWkupPinStatus)
        {
            return true;
        }
    }

    return false;
}