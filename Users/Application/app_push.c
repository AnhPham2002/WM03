#include "app_push.h"
#include "app_protocol.h"

#define PUSH_RESPONSE_TIMEOUT_MS 2000

static Push_Step_t ePushStep = PUSH_STEP_IDLE;
static uint8_t u8RetryCount = 0;
static uint16_t u16LatchIndex = 0;
static uint16_t u16EventIndex = 0;
static bool bWaitingResponse = false;
static uint32_t u32ResponseTime = 0;

static uint8_t au8TxData[512];
static uint16_t u16TxDataLen;
static uint8_t au8RxData[1600];
static uint16_t u16RxDataLen;

void app_push_execute(void)
{
    Protocol_Err_Code_t eErrCode;

    app_cellular_execute();

    if (!app_cellular_get_connection_status())
    {
        ePushStep = PUSH_STEP_IDLE;
        bWaitingResponse = false;
        return;
    }

    switch (ePushStep)
    {
    case PUSH_STEP_IDLE:
        bWaitingResponse = false;
        ePushStep = PUSH_STEP_SEND_INFO;
        break;

    case PUSH_STEP_SEND_INFO:
        if (!bWaitingResponse)
        {
            if (app_protocol_pack_push_info(au8TxData, &u16TxDataLen))
            {
                app_cellular_send_data(au8TxData, u16TxDataLen);
                u32ResponseTime = sys_time_ms();
                bWaitingResponse = true;
            }
            else
            {
                u8RetryCount++;
            }
        }
        else if (app_cellular_receive_data(au8RxData, &u16RxDataLen))
        {
            bWaitingResponse = false;

            eErrCode = app_protocol_process(PROTOCOL_DATA_SOURCE_CELLULAR, au8RxData, u16RxDataLen, au8TxData, &u16TxDataLen);

            if (eErrCode == PROTOCOL_ERR_PUSH_INFO_SUCCESS)
            {
                u8RetryCount = 0;
                ePushStep = PUSH_STEP_SEND_LATCH;
            }
            else if (eErrCode == PROTOCOL_ERR_PUSH_INFO_FAILED)
            {
                u8RetryCount++;
            }
            else
            {
                app_cellular_send_data(au8TxData, u16TxDataLen);
                u32ResponseTime = sys_time_ms();
                bWaitingResponse = true;
            }
        }
        else if (sys_time_ms() - u32ResponseTime >= PUSH_RESPONSE_TIMEOUT_MS)
        {
            bWaitingResponse = false;
            u8RetryCount++;
        }
        break;

    case PUSH_STEP_SEND_LATCH:
        if (!bWaitingResponse)
        {
            if (app_protocol_pack_push_latch(u16LatchIndex, au8TxData, &u16TxDataLen))
            {
                app_cellular_send_data(au8TxData, u16TxDataLen);
                u32ResponseTime = sys_time_ms();
                bWaitingResponse = true;
            }
            else
            {
                ePushStep = PUSH_STEP_SEND_EVENT;
            }
        }
        else if (app_cellular_receive_data(au8RxData, &u16RxDataLen))
        {
            bWaitingResponse = false;

            eErrCode = app_protocol_process(PROTOCOL_DATA_SOURCE_CELLULAR, au8RxData, u16RxDataLen, au8TxData, &u16TxDataLen);

            if (eErrCode == PROTOCOL_ERR_PUSH_LATCH_SUCCESS)
            {
                u16LatchIndex++;
                u8RetryCount = 0;
            }
            else if (eErrCode == PROTOCOL_ERR_PUSH_LATCH_FAILED)
            {
                u8RetryCount++;
            }
            else
            {
                app_cellular_send_data(au8TxData, u16TxDataLen);
                u32ResponseTime = sys_time_ms();
                bWaitingResponse = true;
            }
        }
        else if (sys_time_ms() - u32ResponseTime >= PUSH_RESPONSE_TIMEOUT_MS)
        {
            bWaitingResponse = false;
            u8RetryCount++;
        }
        break;

    case PUSH_STEP_SEND_EVENT:
    {
        uint8_t u8EventPackCount;

        if (!bWaitingResponse)
        {
            if (app_protocol_pack_push_event(u16EventIndex, &u8EventPackCount, au8TxData, &u16TxDataLen))
            {
                app_cellular_send_data(au8TxData, u16TxDataLen);
                u32ResponseTime = sys_time_ms();
                bWaitingResponse = true;
            }
            else
            {
                ePushStep = PUSH_STEP_UPDATE_METADATA;
            }
        }
        else if (app_cellular_receive_data(au8RxData, &u16RxDataLen))
        {
            bWaitingResponse = false;

            eErrCode = app_protocol_process(PROTOCOL_DATA_SOURCE_CELLULAR, au8RxData, u16RxDataLen, au8TxData, &u16TxDataLen);

            if (eErrCode == PROTOCOL_ERR_PUSH_EVENT_SUCCESS)
            {
                u16EventIndex += u8EventPackCount;
                u8RetryCount = 0;
            }
            else if (eErrCode == PROTOCOL_ERR_PUSH_EVENT_FAILED)
            {
                u8RetryCount++;
            }
            else
            {
                app_cellular_send_data(au8TxData, u16TxDataLen);
                u32ResponseTime = sys_time_ms();
                bWaitingResponse = true;
            }
        }
        else if (sys_time_ms() - u32ResponseTime >= PUSH_RESPONSE_TIMEOUT_MS)
        {
            bWaitingResponse = false;
            u8RetryCount++;
        }
        break;
    }

    case PUSH_STEP_UPDATE_METADATA:
    default:
        if ((u16LatchIndex != 0) || (u16EventIndex != 0))
        {
            app_storage_update_load_index(u16LatchIndex, u16EventIndex);
            u16LatchIndex = 0;
            u16EventIndex = 0;
        }

        u8RetryCount = 0;
        bWaitingResponse = false;

        if (app_cellular_receive_data(au8RxData, &u16RxDataLen))
        {
            app_protocol_process(PROTOCOL_DATA_SOURCE_CELLULAR, au8RxData, u16RxDataLen, au8TxData, &u16TxDataLen);
            app_cellular_send_data(au8TxData, u16TxDataLen);
        }

        ePushStep = PUSH_STEP_IDLE;
        app_cellular_stop();
        
        break;
    }

    if (u8RetryCount >= PUSH_RETRY)
    {
        if ((u16LatchIndex != 0) || (u16EventIndex != 0))
        {
            app_storage_update_load_index(u16LatchIndex, u16EventIndex);
            u16LatchIndex = 0;
            u16EventIndex = 0;
        }

        u8RetryCount = 0;
        bWaitingResponse = false;
        ePushStep = PUSH_STEP_IDLE;
        app_cellular_stop();
    }
}