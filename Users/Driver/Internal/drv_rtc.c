#include "drv_rtc.h"
#include "rtc.h"

static volatile uint16_t u16RtcAlarmAIsrCount;

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void drv_rtc_init(void)
{
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    if (HAL_RTC_Init(&hrtc) != HAL_OK)
    {
        Error_Handler();
    }

    RTC_AlarmTypeDef sAlarm = {0};

    sAlarm.AlarmTime.Hours = 0;
    sAlarm.AlarmTime.Minutes = 0;
    sAlarm.AlarmTime.Seconds = 0;
    sAlarm.AlarmTime.SubSeconds = 0;
    sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
    sAlarm.AlarmMask = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
    sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_DATE;
    sAlarm.AlarmDateWeekDay = 1;
    sAlarm.Alarm = RTC_ALARM_A;
    if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }

    if (HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 1023, RTC_WAKEUPCLOCK_RTCCLK_DIV16) != HAL_OK)
    {
        Error_Handler();
    }
}

bool drv_rtc_set_date_time(const Date_Time_t *pDateTime)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    sTime.Hours = pDateTime->u8Hours;
    sTime.Minutes = pDateTime->u8Minutes;
    sTime.Seconds = pDateTime->u8Seconds;
    sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;  // Don't use
    sTime.StoreOperation = RTC_STOREOPERATION_RESET; // Don't use

    sDate.Month = pDateTime->u8Month;
    sDate.Date = pDateTime->u8Date;
    sDate.Year = pDateTime->u8Year;
    sDate.WeekDay = RTC_WEEKDAY_MONDAY; // Don't use

    if ((HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK) || (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK))
    {
        return false;
    }

    u16RtcAlarmAIsrCount = pDateTime->u8Hours * 60 + pDateTime->u8Minutes;
    return true;
}

void drv_rtc_get_date_time(Date_Time_t *pDateTime)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    pDateTime->u8Date = sDate.Date;
    pDateTime->u8Month = sDate.Month;
    pDateTime->u8Year = sDate.Year;
    pDateTime->u8Hours = sTime.Hours;
    pDateTime->u8Minutes = sTime.Minutes;
    pDateTime->u8Seconds = sTime.Seconds;
}

/*==================================================================================================
*                                   INTERRUPT FUNCTIONS
==================================================================================================*/

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    drv_led_blink();
    drv_wdt_refresh();
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
    u16RtcAlarmAIsrCount++;
}
