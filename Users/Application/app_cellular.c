#include "app_cellular.h"
#include "ram_noinit_layout.h"
#include "sv_time.h"

static volatile bool bPushPeriodFlag = false;
static volatile bool *const pDateTimeIsValid = (volatile bool *)RAM_NOINIT_DATE_TIME_IS_VALID_FLAG;

static Cellular_Step_t eCellularStep = CELLULAR_STEP_IDLE;

static uint32_t u32TimeConnect;         // Use for connect network
static uint32_t u32TimeIdleCommunicate; // Use for idle timeout after communicate
static uint32_t u32TimeGetInfo;         // Use for get CCID and signal quality

static bool bConnectedFlag = false;

static const char *const au8NtpServer[] = {
    NTP_SERVER_PRIMARY,
    NTP_SERVER_BACKUP_1,
    NTP_SERVER_BACKUP_2,
};

void app_cellular_init(void)
{
    if (*(volatile uint64_t *)RAM_NOINIT_MAGIC_ADDRESS != RAM_NOINIT_MAGIC_NUMBER)
    {
        *pDateTimeIsValid = false;
    }

    if (!*pDateTimeIsValid)
    {
        eCellularStep = CELLULAR_STEP_ON;
    }

    sv_cellular_init();
}

void app_cellular_start(void)
{
    if (eCellularStep == CELLULAR_STEP_IDLE)
    {
        eCellularStep = CELLULAR_STEP_ON;
    }
}

void app_cellular_execute(void)
{
    static uint8_t u8ResetCount = 0;
    static uint8_t u8RetryCount = 0;
    static uint8_t u8RetryStartSocketCount = 0;
    static uint8_t u8RetryConnectSocketCount = 0;

    switch (eCellularStep)
    {
    case CELLULAR_STEP_IDLE:
        if (bPushPeriodFlag)
        {
            eCellularStep = CELLULAR_STEP_ON;
        }
        break;

    case CELLULAR_STEP_ON:
        if (sv_cellular_on())
        {
            u8RetryCount = 0;
            u32TimeConnect = sys_time_ms();
            eCellularStep = CELLULAR_STEP_WAIT_AT_READY;
        }
        else
        {
            if (u8RetryCount >= CELLULAR_RETRY)
            {
                eCellularStep = CELLULAR_STEP_OFF;
            }
        }
        break;

    case CELLULAR_STEP_WAIT_AT_READY:
        if (sys_time_ms() - u32TimeConnect >= CELLULAR_WAIT_FOR_READY)
        {
            eCellularStep = CELLULAR_STEP_OFF;
        }
        else
        {
            if (sv_cellular_check_at())
            {
                u8RetryCount = 0;
                eCellularStep = CELLULAR_STEP_TURN_OFF_ECHO;
            }
        }
        break;

    case CELLULAR_STEP_TURN_OFF_ECHO:
        if (sv_cellular_off_echo())
        {
            u8RetryCount = 0;
            eCellularStep = CELLULAR_STEP_CHECK_SIM;
        }
        else
        {
            if (++u8RetryCount >= CELLULAR_RETRY)
            {
                eCellularStep = CELLULAR_STEP_OFF;
            }
        }
        break;

    case CELLULAR_STEP_CHECK_SIM:
        if (sv_cellular_check_sim() && (!*pDateTimeIsValid || bPushPeriodFlag))
        {
            u8RetryCount = 0;
            eCellularStep = CELLULAR_STEP_CHECK_REGISTRATION_STATUS;
        }
        else
        {
            if ((++u8RetryCount >= CELLULAR_RETRY) || (sys_time_ms() - u32TimeGetInfo > CELLULAR_GET_INFO_IDLE_TIMEOUT))
            {
                eCellularStep = CELLULAR_STEP_OFF;
            }
        }
        break;

    case CELLULAR_STEP_CHECK_REGISTRATION_STATUS:
        if (sv_cellular_check_registration_status())
        {
            u8RetryCount = 0;
            if (!*pDateTimeIsValid)
            {
                eCellularStep = CELLULAR_STEP_SYNC_DATE_TIME;
            }
            else
            {
                eCellularStep = CELLULAR_STEP_START_SOCKET_SERVICE;
            }
        }
        else
        {
            if (++u8RetryCount >= CELLULAR_RETRY)
            {
                u8RetryCount = 0;
                eCellularStep = CELLULAR_STEP_RESET;
            }
        }
        break;

    case CELLULAR_STEP_SYNC_DATE_TIME:
    {
        static uint8_t u8NtpServerIndex = 0;

        if (sv_cellular_update_system_time(au8NtpServer[u8NtpServerIndex], app_storage_get_timezone()))
        {
            u8RetryCount = 0;
            u8NtpServerIndex = 0;
            eCellularStep = CELLULAR_STEP_GET_DATE_TIME;
        }
        else
        {
            if (++u8RetryCount >= CELLULAR_RETRY)
            {
                u8RetryCount = 0;
                if (++u8NtpServerIndex >= ARRAY_SIZE(au8NtpServer))
                {
                    u8NtpServerIndex = 0;
                    eCellularStep = CELLULAR_STEP_OFF;
                }
            }
        }
        break;
    }

    case CELLULAR_STEP_GET_DATE_TIME:
    {
        Date_Time_t sDateTime;
        if (sv_cellular_get_time(&sDateTime.u8Year, &sDateTime.u8Month, &sDateTime.u8Date, &sDateTime.u8Hours, &sDateTime.u8Minutes, &sDateTime.u8Seconds))
        {
            if (sv_time_set_date_time(&sDateTime))
            {
                *pDateTimeIsValid = true;
                u8RetryCount = 0;
                if (bPushPeriodFlag)
                {
                    eCellularStep = CELLULAR_STEP_CHECK_REGISTRATION_STATUS;
                }
                else
                {
                    eCellularStep = CELLULAR_STEP_OFF;
                }
            }
        }
        if (!*pDateTimeIsValid)
        {
            if (++u8RetryCount >= CELLULAR_RETRY)
            {
                eCellularStep = CELLULAR_STEP_OFF;
            }
        }
        break;
    }

    case CELLULAR_STEP_START_SOCKET_SERVICE:
        if (sv_cellular_start_socket_service())
        {
            u8RetryCount = 0;
            u32TimeConnect = sys_time_ms();
            eCellularStep = CELLULAR_STEP_CHECK_SOCKET_SERVICE;
        }
        else
        {
            if (++u8RetryCount >= CELLULAR_RETRY)
            {
                eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
            }
        }
        break;

    case CELLULAR_STEP_CHECK_SOCKET_SERVICE:
        if (sys_time_ms() - u32TimeConnect < AT_COMMAND_RESPONSE_TIMEOUT_NETWORK)
        {
            bool bStatus;
            if (sv_cellular_check_socket_service(&bStatus))
            {
                u8RetryCount = 0;
                if (bStatus)
                {
                    u8RetryStartSocketCount = 0;
                    eCellularStep = CELLULAR_STEP_CONNECT_TCP_SOCKET;
                }
                else
                {
                    if (++u8RetryStartSocketCount >= CELLULAR_RETRY)
                    {
                        eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
                    }
                    else
                    {
                        eCellularStep = CELLULAR_STEP_START_SOCKET_SERVICE;
                    }
                }
            }
            else
            {
                if (++u8RetryCount >= CELLULAR_RETRY)
                {
                    eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
                }
            }
        }
        else
        {
            if (++u8RetryStartSocketCount >= CELLULAR_RETRY)
            {
                eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
            }
            else
            {
                eCellularStep = CELLULAR_STEP_START_SOCKET_SERVICE;
            }
        }
        break;

    case CELLULAR_STEP_CONNECT_TCP_SOCKET:
    {
        Ip_Endpoint_t sIpEndpoint;
        app_storage_get_ip_endpoint(&sIpEndpoint);
        if (sv_cellular_connect_tcp_socket(sIpEndpoint.au8Ipv4, sIpEndpoint.au8Port))
        {
            u8RetryCount = 0;
            u32TimeConnect = sys_time_ms();
            eCellularStep = CELLULAR_STEP_CHECK_TCP_SOCKET;
        }
        else
        {
            if (++u8RetryCount >= CELLULAR_RETRY)
            {
                eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
            }
        }
        break;
    }

    case CELLULAR_STEP_CHECK_TCP_SOCKET:
        if (sys_time_ms() - u32TimeConnect < AT_COMMAND_RESPONSE_TIMEOUT_NETWORK)
        {
            bool bStatus;
            if (sv_cellular_check_tcp_socket(&bStatus))
            {
                u8RetryCount = 0;
                if (bStatus)
                {
                    u8RetryConnectSocketCount = 0;
                    u32TimeIdleCommunicate = sys_time_ms();
                    eCellularStep = CELLULAR_STEP_COMMUNICATE;
                }
                else
                {
                    if (++u8RetryConnectSocketCount >= CELLULAR_RETRY)
                    {
                        eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
                    }
                    else
                    {
                        eCellularStep = CELLULAR_STEP_CONNECT_TCP_SOCKET;
                    }
                }
            }
            else
            {
                if (++u8RetryCount >= CELLULAR_RETRY)
                {
                    eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
                }
            }
        }
        else
        {
            if (++u8RetryConnectSocketCount >= CELLULAR_RETRY)
            {
                eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
            }
            else
            {
                eCellularStep = CELLULAR_STEP_CONNECT_TCP_SOCKET;
            }
        }
        break;

    case CELLULAR_STEP_COMMUNICATE:
        bConnectedFlag = true;
        if (sys_time_ms() - u32TimeIdleCommunicate >= CELLULAR_COMMUNICATE_IDLE_TIMEOUT)
        {
            eCellularStep = CELLULAR_STEP_CLOSE_TCP_SOCKET;
        }
        break;

    case CELLULAR_STEP_CLOSE_TCP_SOCKET:
        bConnectedFlag = false;
        sv_cellular_close_tcp_socket();
        eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
        break;

    case CELLULAR_STEP_STOP_SOCKET_SERVICE:
        bConnectedFlag = false;
        sv_cellular_stop_socket_service();
        eCellularStep = CELLULAR_STEP_OFF;
        break;

    case CELLULAR_STEP_RESET:
        bConnectedFlag = false;
        if (++u8ResetCount >= CELLULAR_RETRY)
        {
            eCellularStep = CELLULAR_STEP_OFF;
        }
        else
        {
            sv_cellular_reset();
            u32TimeConnect = sys_time_ms();
            eCellularStep = CELLULAR_STEP_WAIT_AT_READY;
        }
        break;

    case CELLULAR_STEP_OFF:
    default:
        sv_cellular_off();
        bPushPeriodFlag = false;
        bConnectedFlag = false;
        u8RetryCount = 0;
        u8ResetCount = 0;
        u8RetryStartSocketCount = 0;
        u8RetryConnectSocketCount = 0;
        eCellularStep = CELLULAR_STEP_IDLE;
        break;
    }
}

void app_cellular_push_activate(void)
{
    bPushPeriodFlag = true;
}

Task_Status_t app_cellular_get_ccid(uint8_t *pCcid, uint8_t *u8CcidLen)
{
    if (eCellularStep == CELLULAR_STEP_IDLE)
    {
        app_cellular_start();
        return TASK_STATUS_RUNNING;
    }

    if ((eCellularStep == CELLULAR_STEP_ON) || (eCellularStep == CELLULAR_STEP_WAIT_AT_READY))
    {
        return TASK_STATUS_RUNNING;
    }

    u32TimeGetInfo = sys_time_ms();
    if (sv_cellular_get_ccid(pCcid, u8CcidLen))
    {
        return TASK_STATUS_SUCCESS;
    }

    return TASK_STATUS_FAILED;
}

Task_Status_t app_cellular_get_signal_quality(int8_t *s8Rssi, int8_t *s8Rsrp, int8_t *s8Rsrq, int8_t *s8Rssnr)
{
    if (eCellularStep == CELLULAR_STEP_IDLE)
    {
        app_cellular_start();
        return TASK_STATUS_RUNNING;
    }

    if ((eCellularStep == CELLULAR_STEP_ON) || (eCellularStep == CELLULAR_STEP_WAIT_AT_READY))
    {
        return TASK_STATUS_RUNNING;
    }

    u32TimeGetInfo = sys_time_ms();
    if (sv_cellular_check_signal_quality(s8Rssi, s8Rsrp, s8Rsrq, s8Rssnr))
    {
        return TASK_STATUS_SUCCESS;
    }

    return TASK_STATUS_FAILED;
}

bool app_cellular_send_data(const uint8_t *pData, uint16_t u16Len)
{
    if ((pData == NULL) || (u16Len == 0))
    {
        return false;
    }

    if (bConnectedFlag)
    {
        if (sv_cellular_send_data(pData, u16Len))
        {
            u32TimeIdleCommunicate = sys_time_ms();
            return true;
        }
        else
        {
            eCellularStep = CELLULAR_STEP_RESET;
            return false;
        }
    }

    return false;
}

bool app_cellular_receive_data(uint8_t *pData, uint16_t *u16Len)
{
	if ((pData == NULL) || (u16Len == NULL))
	{
		return false;
	}

	if (bConnectedFlag)
	{
		if (sv_cellular_receive_data(pData, u16Len))
		{
			u32TimeIdleCommunicate = sys_time_ms();
			return true;
		}
	}

	return false;
}

bool app_cellular_get_connection_status(void)
{
	return bConnectedFlag;
}