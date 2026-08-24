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

uint32_t sv_time_get_unix_timestamp(void)
{
    Date_Time_t sDateTime;
    uint32_t u32Days = 0U;
    uint16_t u16Year;
    uint32_t u32Timestamp;

    sv_time_get_date_time(&sDateTime);

    u16Year = 2000U + sDateTime.u8Year;

    for (uint16_t u16YearTemp = 1970U; u16YearTemp < u16Year; u16YearTemp++)
    {
        u32Days += ((u16YearTemp % 4U == 0U) && ((u16YearTemp % 100U != 0U) || (u16YearTemp % 400U == 0U))) ? 366U : 365U;
    }

    static const uint8_t au8DaysInMonth[] = {31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};

    for (uint8_t u8Month = 1U; u8Month < sDateTime.u8Month; u8Month++)
    {
        u32Days += au8DaysInMonth[u8Month - 1U];

        if ((u8Month == 2U) && (u16Year % 4U == 0U) && ((u16Year % 100U != 0U) || (u16Year % 400U == 0U)))
        {
            u32Days++;
        }
    }

    u32Days += sDateTime.u8Date - 1U;

    u32Timestamp = u32Days * 86400UL;
    u32Timestamp += sDateTime.u8Hours * 3600UL;
    u32Timestamp += sDateTime.u8Minutes * 60UL;
    u32Timestamp += sDateTime.u8Seconds;

    return u32Timestamp;
}