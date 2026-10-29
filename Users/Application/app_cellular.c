#include "app_cellular.h"

static volatile bool bPushPeriodFlag = false;
static uint32_t u32TimeStartPushSession;

static bool bCellularResultReady = false;
static Cellular_Error_t eCellularError = CELLULAR_ERROR_NONE;

static Cellular_Step_t eCellularStep = CELLULAR_STEP_IDLE;
static uint8_t u8ResetCount = 0;
static uint8_t u8RetryCount = 0;
static uint8_t u8RetryStartSocketCount = 0;
static uint8_t u8RetryConnectSocketCount = 0;

static bool bCcidInfoRequested = false;
static bool bCcidInfoValid = false;
static Ccid_Info_t sCcidInfo;

static bool bSignalInfoRequested = false;
static bool bSignalInfoValid = false;
static Signal_Info_t sSignalInfo;

static uint32_t u32TimeConnect; // Use for connect network

static bool bConnectedFlag = false;

static const char *const au8NtpServer[] = {
    NTP_SERVER_PRIMARY,
    NTP_SERVER_BACKUP_1,
    NTP_SERVER_BACKUP_2,
};

void app_cellular_init(void)
{
    sv_cellular_init();

    if (sv_time_get_date_time_update_required())
    {
        eCellularStep = CELLULAR_STEP_ON;
    }
}

void app_cellular_start(void)
{
    if (eCellularStep == CELLULAR_STEP_IDLE)
    {
        eCellularError = CELLULAR_ERROR_NONE;
        bCellularResultReady = false;
        bConnectedFlag = false;
        eCellularStep = CELLULAR_STEP_ON;
    }
}

void app_cellular_stop(void)
{
    eCellularStep = CELLULAR_STEP_CLOSE_TCP_SOCKET;
}

void app_cellular_execute(void)
{
    switch (eCellularStep)
    {
    case CELLULAR_STEP_IDLE:
        if (bPushPeriodFlag)
        {
            eCellularStep = CELLULAR_STEP_ON;
        }
        break;

    case CELLULAR_STEP_ON:
        Cellular_Task_Status_t eTaskStatus = sv_cellular_on();
        if (eTaskStatus != CELLULAR_TASK_WAIT)
        {
            if (eTaskStatus == CELLULAR_TASK_DONE)
            {
                eCellularStep = CELLULAR_STEP_WAIT_AT_READY;
            }
            else
            {
                eCellularError = CELLULAR_ERROR_POWER_ON;
                eCellularStep = CELLULAR_STEP_OFF;
            }
        }
        break;

    case CELLULAR_STEP_WAIT_AT_READY:
    {
        Cellular_Task_Status_t eTaskStatus = sv_cellular_check_at();
        if (eTaskStatus != CELLULAR_TASK_WAIT)
        {
            if (eTaskStatus == CELLULAR_TASK_DONE)
            {
                u8RetryCount = 0;
                eCellularStep = CELLULAR_STEP_TURN_OFF_ECHO;
            }
            else
            {
                if (++u8RetryCount >= CELLULAR_RETRY)
                {
                    eCellularError = CELLULAR_ERROR_AT_READY;
                    eCellularStep = CELLULAR_STEP_OFF;
                }
            }
        }
        break;
    }

    case CELLULAR_STEP_TURN_OFF_ECHO:
    {
        Cellular_Task_Status_t eTaskStatus = sv_cellular_off_echo();
        if (eTaskStatus != CELLULAR_TASK_WAIT)
        {
            if (eTaskStatus == CELLULAR_TASK_DONE)
            {
                u8RetryCount = 0;
                eCellularStep = CELLULAR_STEP_CHECK_SIM;
            }
            else
            {
                if (++u8RetryCount >= CELLULAR_RETRY)
                {
                    eCellularError = CELLULAR_ERROR_AT_READY;
                    eCellularStep = CELLULAR_STEP_OFF;
                }
            }
        }
        break;
    }

    case CELLULAR_STEP_CHECK_SIM:
    {
        Cellular_Task_Status_t eTaskStatus = sv_cellular_check_sim();
        if (eTaskStatus != CELLULAR_TASK_WAIT)
        {
            if (eTaskStatus == CELLULAR_TASK_DONE)
            {
                u8RetryCount = 0;
                eCellularStep = CELLULAR_STEP_GET_INFO;
            }
            else
            {
                if (++u8RetryCount >= CELLULAR_RETRY)
                {
                    eCellularError = CELLULAR_ERROR_SIM_NOT_READY;
                    eCellularStep = CELLULAR_STEP_OFF;
                }
            }
        }
        break;
    }

    case CELLULAR_STEP_GET_INFO:
        if (!bCcidInfoRequested)
        {
            Cellular_Task_Status_t eTaskStatus = sv_cellular_get_ccid(&sCcidInfo);
            if (eTaskStatus != CELLULAR_TASK_WAIT)
            {
                if (eTaskStatus == CELLULAR_TASK_DONE)
                {
                    u8RetryCount = 0;
                    bCcidInfoValid = true;
                    bCcidInfoRequested = true;
                }
                else
                {
                    if (++u8RetryCount >= CELLULAR_RETRY)
                    {
                        u8RetryCount = 0;
                        bCcidInfoRequested = true;
                    }
                }
            }
        }
        else if (!bSignalInfoRequested)
        {
            Cellular_Task_Status_t eTaskStatus = sv_cellular_check_signal_quality(&sSignalInfo);
            if (eTaskStatus != CELLULAR_TASK_WAIT)
            {
                if (eTaskStatus == CELLULAR_TASK_DONE)
                {
                    u8RetryCount = 0;
                    bSignalInfoValid = true;
                    bSignalInfoRequested = true;
                }
                else
                {
                    if (++u8RetryCount >= CELLULAR_RETRY)
                    {
                        u8RetryCount = 0;
                        bSignalInfoRequested = true;
                    }
                }

                bSignalInfoValid = true;
                bSignalInfoRequested = true;
            }
        }

        if (bCcidInfoRequested && bSignalInfoRequested)
        {
            if (sv_time_get_date_time_update_required() || bPushPeriodFlag)
            {
                u8RetryCount = 0;
                eCellularStep = CELLULAR_STEP_CHECK_REGISTRATION_STATUS;
            }
            else
            {
                eCellularStep = CELLULAR_STEP_OFF;
            }
        }

        break;

    case CELLULAR_STEP_CHECK_REGISTRATION_STATUS:
    {
        Cellular_Task_Status_t eTaskStatus = sv_cellular_check_registration_status();
        if (eTaskStatus != CELLULAR_TASK_WAIT)
        {
            if (eTaskStatus == CELLULAR_TASK_DONE)
            {
                u8RetryCount = 0;
                if (sv_time_get_date_time_update_required())
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
                    eCellularError = CELLULAR_ERROR_NETWORK_NOT_REGISTERED;
                    eCellularStep = CELLULAR_STEP_OFF;
                }
            }
        }
        break;
    }

    case CELLULAR_STEP_SYNC_DATE_TIME:
    {
        static uint8_t u8NtpServerIndex = 0;
        Cellular_Task_Status_t eTaskStatus = sv_cellular_update_system_time(au8NtpServer[u8NtpServerIndex], app_storage_get_timezone());
        if (eTaskStatus != CELLULAR_TASK_WAIT)
        {
            if (eTaskStatus == CELLULAR_TASK_DONE)
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
                        eCellularError = CELLULAR_ERROR_DATE_TIME_SYNC;
                        u8NtpServerIndex = 0;
                        eCellularStep = CELLULAR_STEP_OFF;
                    }
                }
            }
        }
        break;
    }

    case CELLULAR_STEP_GET_DATE_TIME:
    {
        Date_Time_t sDateTime;
        Cellular_Task_Status_t eTaskStatus = sv_cellular_get_time(&sDateTime.u8Year, &sDateTime.u8Month, &sDateTime.u8Date, &sDateTime.u8Hours, &sDateTime.u8Minutes, &sDateTime.u8Seconds);
        if (eTaskStatus != CELLULAR_TASK_WAIT)
        {
            if (eTaskStatus == CELLULAR_TASK_DONE)
            {
                if (sv_time_set_date_time(&sDateTime))
                {
                    u8RetryCount = 0;
                    if (bPushPeriodFlag)
                    {
                        eCellularStep = CELLULAR_STEP_CHECK_REGISTRATION_STATUS;
                    }
                    else
                    {
                        eCellularError = CELLULAR_ERROR_NONE;
                        eCellularStep = CELLULAR_STEP_OFF;
                    }
                }
            }
            if (sv_time_get_date_time_update_required())
            {
                if (++u8RetryCount >= CELLULAR_RETRY)
                {
                    eCellularError = CELLULAR_ERROR_GET_DATE_TIME;
                    eCellularStep = CELLULAR_STEP_OFF;
                }
            }
        }
        break;
    }

    case CELLULAR_STEP_START_SOCKET_SERVICE:
    {
        Cellular_Task_Status_t eTaskStatus = sv_cellular_start_socket_service();
        if (eTaskStatus != CELLULAR_TASK_WAIT)
        {
            if (eTaskStatus == CELLULAR_TASK_DONE)
            {
                u8RetryCount = 0;
                u32TimeConnect = sys_time_ms();
                eCellularStep = CELLULAR_STEP_CHECK_SOCKET_SERVICE;
            }
            else
            {
                if (++u8RetryCount >= CELLULAR_RETRY)
                {
                    eCellularError = CELLULAR_ERROR_SOCKET_SERVICE_NOT_READY;
                    eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
                }
            }
        }
        break;
    }

    case CELLULAR_STEP_CHECK_SOCKET_SERVICE:
    {
        if (sys_time_ms() - u32TimeConnect < AT_COMMAND_RESPONSE_TIMEOUT_NETWORK)
        {
            bool bStatus;
            Cellular_Task_Status_t eTaskStatus = sv_cellular_check_socket_service(&bStatus);
            if (eTaskStatus != CELLULAR_TASK_WAIT)
            {
                if (eTaskStatus == CELLULAR_TASK_DONE)
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
                            eCellularError = CELLULAR_ERROR_SOCKET_SERVICE_NOT_READY;
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
                        eCellularError = CELLULAR_ERROR_SOCKET_SERVICE_NOT_READY;
                        eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
                    }
                }
            }
        }
        else
        {
            if (++u8RetryStartSocketCount >= CELLULAR_RETRY)
            {
                eCellularError = CELLULAR_ERROR_SOCKET_SERVICE_NOT_READY;
                eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
            }
            else
            {
                eCellularStep = CELLULAR_STEP_START_SOCKET_SERVICE;
            }
        }
        break;
    }

    case CELLULAR_STEP_CONNECT_TCP_SOCKET:
    {
        Ip_Endpoint_t sIpEndpoint;
        app_storage_get_ip_endpoint(&sIpEndpoint);
        Cellular_Task_Status_t eTaskStatus = sv_cellular_connect_tcp_socket(sIpEndpoint.au8Ipv4, sIpEndpoint.au8Port);
        if (eTaskStatus != CELLULAR_TASK_WAIT)
        {
            if (eTaskStatus == CELLULAR_TASK_DONE)
            {
                u8RetryCount = 0;
                u32TimeConnect = sys_time_ms();
                eCellularStep = CELLULAR_STEP_CHECK_TCP_SOCKET;
            }
            else
            {
                if (++u8RetryCount >= CELLULAR_RETRY)
                {
                    eCellularError = CELLULAR_ERROR_TCP_CONNECT;
                    eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
                }
            }
        }
        break;
    }

    case CELLULAR_STEP_CHECK_TCP_SOCKET:
        if (sys_time_ms() - u32TimeConnect < AT_COMMAND_RESPONSE_TIMEOUT_NETWORK)
        {
            bool bStatus;
            Cellular_Task_Status_t eTaskStatus = sv_cellular_check_tcp_socket(&bStatus);
            if (eTaskStatus != CELLULAR_TASK_WAIT)
            {
                if (eTaskStatus == CELLULAR_TASK_DONE)
                {
                    u8RetryCount = 0;
                    if (bStatus)
                    {
                        u8RetryConnectSocketCount = 0;
                        eCellularStep = CELLULAR_STEP_COMMUNICATE;
                    }
                    else
                    {
                        if (++u8RetryConnectSocketCount >= CELLULAR_RETRY)
                        {
                            eCellularError = CELLULAR_ERROR_TCP_CONNECT;
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
                        eCellularError = CELLULAR_ERROR_TCP_CONNECT;
                        eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
                    }
                }
            }
        }
        else
        {
            if (++u8RetryConnectSocketCount >= CELLULAR_RETRY)
            {
                eCellularError = CELLULAR_ERROR_TCP_CONNECT;
                eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
            }
            else
            {
                eCellularStep = CELLULAR_STEP_CONNECT_TCP_SOCKET;
            }
        }
        break;

    case CELLULAR_STEP_COMMUNICATE:
        eCellularError = CELLULAR_ERROR_NONE;
        bConnectedFlag = true;
        break;

    case CELLULAR_STEP_CLOSE_TCP_SOCKET:
        bConnectedFlag = false;
        if (sv_cellular_close_tcp_socket() != CELLULAR_TASK_WAIT)
        {
            eCellularStep = CELLULAR_STEP_STOP_SOCKET_SERVICE;
        }
        break;

    case CELLULAR_STEP_STOP_SOCKET_SERVICE:
        bConnectedFlag = false;
        if (sv_cellular_stop_socket_service() != CELLULAR_TASK_WAIT)
        {
            eCellularStep = CELLULAR_STEP_OFF;
        }
        break;

    case CELLULAR_STEP_RESET:
        bConnectedFlag = false;
        if (++u8ResetCount >= CELLULAR_RETRY)
        {
            eCellularStep = CELLULAR_STEP_OFF;
        }
        else
        {
            if (sv_cellular_reset() == CELLULAR_TASK_DONE)
            {
                u32TimeConnect = sys_time_ms();
                eCellularStep = CELLULAR_STEP_WAIT_AT_READY;
            }
        }
        break;

    case CELLULAR_STEP_OFF:
        if (sv_cellular_off() == CELLULAR_TASK_DONE)
        {
            bCcidInfoRequested = false;
            bCcidInfoValid = false;
            bSignalInfoRequested = false;
            bSignalInfoValid = false;
            if (bPushPeriodFlag)
            {
                bCellularResultReady = true;
            }
            bPushPeriodFlag = false;
            bConnectedFlag = false;
            u8RetryCount = 0;
            u8ResetCount = 0;
            u8RetryStartSocketCount = 0;
            u8RetryConnectSocketCount = 0;
            eCellularStep = CELLULAR_STEP_IDLE;
        }
        break;

    default:
        if (sv_cellular_off() == CELLULAR_TASK_DONE)
        {
            bCcidInfoRequested = false;
            bCcidInfoValid = false;
            bSignalInfoRequested = false;
            bSignalInfoValid = false;
            bCellularResultReady = false;
            bPushPeriodFlag = false;
            bConnectedFlag = false;
            u8RetryCount = 0;
            u8ResetCount = 0;
            u8RetryStartSocketCount = 0;
            u8RetryConnectSocketCount = 0;
            eCellularStep = CELLULAR_STEP_IDLE;
        }
        break;
    }
}

void app_cellular_push_activate(void)
{
    u32TimeStartPushSession = sys_time_ms();
    bPushPeriodFlag = true;
}

bool app_cellular_get_ccid(Ccid_Info_t *pInfo)
{
    if (pInfo == NULL)
    {
        return false;
    }

    if (bCcidInfoValid)
    {
        memcpy(pInfo, &sCcidInfo, sizeof(Ccid_Info_t));
    }
    else
    {
        memset(pInfo, 0, sizeof(Ccid_Info_t));
    }
    return bCcidInfoValid;
}

bool app_cellular_get_signal_quality(Signal_Info_t *pInfo)
{
    if (pInfo == NULL)
    {
        return false;
    }

    if (bSignalInfoValid)
    {
        memcpy(pInfo, &sSignalInfo, sizeof(Signal_Info_t));
    }
    else
    {
        memset(pInfo, 0, sizeof(Signal_Info_t));
    }
    return bSignalInfoValid;
}

bool app_cellular_send_data(const uint8_t *pData, uint16_t u16Len)
{
    if ((pData == NULL) || (u16Len == 0))
    {
        return false;
    }

    if (bConnectedFlag)
    {
        Cellular_Task_Status_t eTaskStatus;
        do
        {
            eTaskStatus = sv_cellular_send_data(pData, u16Len);
        } while (eTaskStatus == CELLULAR_TASK_WAIT);

        if (eTaskStatus == CELLULAR_TASK_WAIT)
        {
            return true;
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
        Cellular_Task_Status_t eTaskStatus;
        do
        {
            eTaskStatus = sv_cellular_receive_data(pData, u16Len);
        } while (eTaskStatus == CELLULAR_TASK_WAIT);

        if (eTaskStatus == CELLULAR_TASK_WAIT)
        {
            return true;
        }
    }

    return false;
}

bool app_cellular_get_connection_status(void)
{
    return bConnectedFlag;
}

bool app_cellular_get_error(Cellular_Error_t *pErr)
{
    if (pErr == NULL)
    {
        return false;
    }

    if (!bCellularResultReady)
    {
        return false;
    }

    *pErr = eCellularError;
    eCellularError = CELLULAR_ERROR_NONE;
    bCellularResultReady = false;

    return true;
}

uint32_t app_cellular_get_push_session_time(void)
{
    return (sys_time_ms() - u32TimeStartPushSession + 500) / 1000;
}

bool app_cellular_communication_in_progress(void)
{
    return eCellularStep != CELLULAR_STEP_IDLE;
}