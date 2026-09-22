#include "app_push.h"
#include "app_protocol.h"

#define RAM_NOINIT_PUSH_MAGIC_NUMBER 0x12344321
#define PUSH_RESPONSE_TIMEOUT_MS 2000

typedef struct
{
    uint32_t u32Magic;
    bool bPushWaiting;
    uint32_t u32TriggerTimestamp;
} Push_Status_t;

static volatile Push_Status_t sPushStatus RAM_NOINIT;

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

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Generate a randomized push delay.
 *
 * Generates a deterministic delay based on the module serial number
 * within the configured push period.
 *
 * @return Randomized push delay in seconds.
 */
static uint32_t app_push_get_random_push_delay(void);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void app_push_execute(void)
{
    Protocol_Err_Code_t eErrCode;

    if (sPushStatus.u32Magic != RAM_NOINIT_PUSH_MAGIC_NUMBER)
    {
        sPushStatus.bPushWaiting = false;
        sPushStatus.u32Magic = RAM_NOINIT_PUSH_MAGIC_NUMBER;
    }

    if (sPushStatus.bPushWaiting)
    {
        if ((sv_time_get_unix_timestamp() - sPushStatus.u32TriggerTimestamp) > app_push_get_random_push_delay())
        {
            app_cellular_push_activate();
            sPushStatus.bPushWaiting = false;
        }
    }

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

void app_push_activate(void)
{
    sPushStatus.bPushWaiting = true;
    sPushStatus.u32TriggerTimestamp = sv_time_get_unix_timestamp();
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static uint32_t app_push_get_random_push_delay(void)
{
    uint64_t u64Serial = app_storage_get_module_serial();
    uint16_t u16PushPeriod = app_storage_get_push_period();
    uint32_t u32MaxDelay;
    uint32_t u32Seed;

    if (u16PushPeriod < 60U)
    {
        u32MaxDelay = (uint32_t)u16PushPeriod * 60U;
    }
    else
    {
        u32MaxDelay = 3600U;
    }

    u32Seed = (uint32_t)(u64Serial ^ (u64Serial >> 32));

    return u32Seed % u32MaxDelay;
}