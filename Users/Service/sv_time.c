#include "sv_time.h"

void sv_time_init(void)
{
    drv_rtc_init();
}

bool sv_time_set_date_time(const Date_Time_t *pDateTime)
{
    return drv_rtc_set_date_time(pDateTime);
}

void sv_time_get_date_time(Date_Time_t *pDateTime)
{
    drv_rtc_get_date_time(pDateTime);
}