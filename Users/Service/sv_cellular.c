#include "sv_cellular.h"

static Cellular_Wait_Context_t sContext;

static uint8_t au8RxBuf[CELLULAR_BUFFER_SIZE];
static uint16_t u16RxBufSize;

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Clear cellular module receive buffer.
 */
static void sv_cellular_flush_buffer(void);

/**
 * @brief Start waiting for cellular response keywords.
 *
 * Initializes the wait context with the specified keywords and timeout.
 *
 * @param[out] pCtx           Wait context.
 * @param[in]  pKeywords      Keyword list.
 * @param[in]  u8KeywordCount Number of keywords.
 * @param[in]  u32Timeout     Timeout in milliseconds.
 */
static void sv_cellular_wait_for_keywords_start(Cellular_Wait_Context_t *pCtx, const char *const *pKeywords, uint8_t u8KeywordCount, uint32_t u32Timeout);

/**
 * @brief Poll for cellular response keywords.
 *
 * Checks received data for the specified keywords and monitors the timeout.
 *
 * @param[in,out] pCtx Wait context.
 *
 * @return Wait result indicating success, pending, timeout, or error.
 */
static Cellular_Wait_Result_t sv_cellular_wait_for_keywords_poll(Cellular_Wait_Context_t *pCtx);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void sv_cellular_init(void)
{
    drv_cellular_init();
}

Cellular_Task_Status_t sv_cellular_on(void)
{
    static bool bTaskStarted = false;
    static uint32_t u32TimeStart;
    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();
        drv_cellular_pwr_on();
        sys_delay_ms(50); // Wait for charge capacitor
        drv_cellular_turn_on();
        u32TimeStart = sys_time_ms();
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        if (sys_time_ms() - u32TimeStart > CELLULAR_WAIT_FOR_READY) // Wait for UART ready
        {
            bTaskStarted = false;
            return CELLULAR_TASK_DONE;
        }
        return CELLULAR_TASK_WAIT;
    }
}

Cellular_Task_Status_t sv_cellular_off(void)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\nOK\r\n", "POWERED DOWN"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+CPOF\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            if (eResult != CELLULAR_WAIT_OK)
            {
                drv_cellular_turn_off(); // Hard off
            }

            sys_delay_ms(100); // Delay for safety shutdown
            drv_cellular_pwr_off();
            bTaskStarted = false;
            return CELLULAR_TASK_DONE;
        }
    }
}

Cellular_Task_Status_t sv_cellular_reset(void)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\nOK\r\n", "\r\nQCRDY\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+CRESET\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                return CELLULAR_TASK_DONE;
            }
            else
            {
                drv_cellular_reset();
                return CELLULAR_TASK_DONE;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_check_at(void)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\nOK\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                return CELLULAR_TASK_DONE;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_off_echo(void)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\nOK\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "ATE0\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                return CELLULAR_TASK_DONE;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_check_sim(void)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\n+CPIN: READY\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+CPIN?\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                return CELLULAR_TASK_DONE;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_check_registration_status(void)
{
    static bool bTaskStarted = false;
    /*
     * 1 registered, home network
     * 5 registered, roaming
     */
    static const char *const pKeywords[] = {"\r\n+CEREG: 0,1\r\n", "\r\n+CEREG: 0,5\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+CEREG?\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                return CELLULAR_TASK_DONE;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_start_socket_service(void)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\n+NETOPEN: 0\r\n", "\r\n+IP ERROR: Network is already opened\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+NETOPEN\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                return CELLULAR_TASK_DONE;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_stop_socket_service(void)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\n+NETCLOSE: 0\r\n", "\r\n+NETCLOSE: 2\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+NETCLOSE\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                return CELLULAR_TASK_DONE;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_check_socket_service(bool *bStatus)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\nOK\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+NETOPEN?\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                char *p = strstr((char *)au8RxBuf, "+NETOPEN:");
                if (p)
                {
                    p += 9; // Skip "+NETOPEN:"
                    while (*p == ' ')
                    {
                        p++;
                    }

                    if ((*p == '0') || (*p == '1'))
                    {
                        *bStatus = (bool)(*p - '0');
                        return CELLULAR_TASK_DONE;
                    }
                }

                return CELLULAR_TASK_FAIL;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_connect_tcp_socket(const uint8_t *pIp, uint8_t *pPort)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\nOK\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        char pIpStr[16];
        memcpy(pIpStr, pIp, 15);
        pIpStr[15] = '\0';

        char au8AtCommand[64] = {0};
        snprintf(au8AtCommand, sizeof(au8AtCommand), "AT+CIPOPEN=0,\"TCP\",\"%s\",%s\r\n", pIpStr, pPort);
        drv_cellular_send((const uint8_t *)au8AtCommand, strlen(au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                return CELLULAR_TASK_DONE;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_close_tcp_socket(void)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\n+CIPCLOSE: 0,0\r\n", "\r\n+CIPCLOSE: 0,4\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+CIPCLOSE=0\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                return CELLULAR_TASK_DONE;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_check_tcp_socket(bool *bStatus)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\nOK\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+CIPCLOSE?\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                char *p = strstr((char *)au8RxBuf, "+CIPCLOSE:");
                if (p)
                {
                    p += 10; // Skip "+CIPCLOSE:"
                    while (*p == ' ')
                    {
                        p++;
                    }

                    if ((*p == '0') || (*p == '1'))
                    {
                        *bStatus = (bool)(*p - '0');
                        return CELLULAR_TASK_DONE;
                    }
                }

                return CELLULAR_TASK_FAIL;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_send_data(const uint8_t *pData, uint16_t u16Size)
{
    typedef enum
    {
        SEND_IDLE = 0,
        SEND_WAIT_PROMPT,
        SEND_WAIT_CONFIRM
    } Send_Status_t;

    static Send_Status_t eSendStatus = SEND_IDLE;
    static const char *const pKeywords1[] = {">"};
    static const char *const pKeywords2[] = {"+CIPSEND:"};

    if (eSendStatus == SEND_IDLE)
    {
        if ((pData == NULL) || (u16Size == 0))
        {
            return CELLULAR_TASK_FAIL;
        }

        sv_cellular_flush_buffer();

        char au8AtCommand[32] = {0};
        snprintf(au8AtCommand, sizeof(au8AtCommand), "AT+CIPSEND=0,%u\r\n", (unsigned int)u16Size);
        drv_cellular_send((const uint8_t *)au8AtCommand, strlen(au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords1, ARRAY_SIZE(pKeywords1), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);

        eSendStatus = SEND_WAIT_PROMPT;
        return CELLULAR_TASK_WAIT;
    }

    if (eSendStatus == SEND_WAIT_PROMPT)
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else if (eResult == CELLULAR_WAIT_OK)
        {
            sv_cellular_flush_buffer();
            drv_cellular_send(pData, u16Size);
            static const uint8_t u8CtrlZ = 0x1A;
            drv_cellular_send(&u8CtrlZ, 1);

            sv_cellular_wait_for_keywords_start(&sContext, pKeywords2, ARRAY_SIZE(pKeywords2), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);

            eSendStatus = SEND_WAIT_CONFIRM;
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            eSendStatus = SEND_IDLE;
            return CELLULAR_TASK_FAIL;
        }
    }

    if (eSendStatus == SEND_WAIT_CONFIRM)
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            eSendStatus = SEND_IDLE;

            if (eResult == CELLULAR_WAIT_OK)
            {
                char *p = strchr((char *)au8RxBuf, ','); // Find the first comma
                if (p != NULL)
                {
                    p++; // Skip ','
                    uint16_t u16RequestedSendLen = (uint16_t)atoi(p);

                    if (u16Size == u16RequestedSendLen)
                    {
                        p = strchr(p, ','); // Find the second comma
                        if (p != NULL)
                        {
                            p++; // Skip ','
                            uint16_t u16ConfirmedSendLen = (uint16_t)atoi(p);

                            if (u16RequestedSendLen == u16ConfirmedSendLen)
                            {
                                return CELLULAR_TASK_DONE;
                            }
                        }
                    }
                }

                return CELLULAR_TASK_FAIL;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }

    eSendStatus = SEND_IDLE;
    return CELLULAR_TASK_FAIL;
}

Cellular_Task_Status_t sv_cellular_receive_data(uint8_t *pData, uint16_t *u16Size)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"+IPD"};

    if ((pData == NULL) || (u16Size == NULL))
    {
        return CELLULAR_TASK_FAIL;
    }

    if (!bTaskStarted)
    {
        char *p = strstr((char *)au8RxBuf, "+IPD");
        if (p != NULL)
        {
            p += 4; // Skip "+IPD"
            *u16Size = (uint16_t)atoi(p);

            p = strstr(p, "\r\n");
            if (p != NULL)
            {
                p += 2; // Skip "\r\n"
                memcpy(pData, p, *u16Size);
                sv_cellular_flush_buffer();
                return CELLULAR_TASK_DONE;
            }

            return CELLULAR_TASK_FAIL;
        }

        bTaskStarted = true;
        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), HES_RESPONSE_TIMEOUT);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                char *p = strstr((char *)au8RxBuf, "+IPD");
                if (p != NULL)
                {
                    p += 4; // Skip "+IPD"
                    *u16Size = (uint16_t)atoi(p);

                    p = strstr(p, "\r\n");
                    if (p != NULL)
                    {
                        p += 2; // Skip "\r\n"
                        memcpy(pData, p, *u16Size);
                        sv_cellular_flush_buffer();
                        return CELLULAR_TASK_DONE;
                    }
                }

                return CELLULAR_TASK_FAIL;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_get_ccid(Ccid_Info_t *pInfo)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\nOK\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+CICCID\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                char *p = strstr((char *)au8RxBuf, "+ICCID:");
                if (p)
                {
                    p += 7; // Skip "+ICCID:"
                    while (*p == ' ')
                    {
                        p++;
                    }

                    char *pEnd = strchr(p, '\r');
                    if (pEnd)
                    {
                        pInfo->u8CcidLen = pEnd - p;
                        if (pInfo->u8CcidLen > MAX_SIM_CCID_SIZE)
                        {
                            return CELLULAR_TASK_FAIL;
                        }
                        memcpy(pInfo->au8Ccid, p, pInfo->u8CcidLen);
                        return CELLULAR_TASK_DONE;
                    }
                }

                return CELLULAR_TASK_FAIL;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_check_signal_quality(Signal_Info_t *pInfo)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\nOK\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+CPSI?\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                const char *p;
                const char *pFieldStart;
                char *pEnd;
                uint8_t u8FieldIndex = 0;
                int sValue;

                p = strchr((char *)au8RxBuf, ':');
                if (p == NULL)
                {
                    return CELLULAR_TASK_FAIL;
                }

                p++; // Move to the first field after ':'
                pFieldStart = p;

                /*
                 * +CPSI: <System Mode>,<Operation Mode>
                 * [,<MCC>-<MNC>,<TAC>,<SCellID>,<PCellID>,
                 * <Frequency Band>,<earfcn>,<dlbw>,<ulbw>,
                 * <RSRQ>,<RSRP>,<RSSI>,<RSSNR>]
                 */
                while (1)
                {
                    if ((*p == ',') || (*p == '\0') || (*p == '\r') || (*p == '\n'))
                    {
                        if (u8FieldIndex == 10) // RSRQ
                        {
                            sValue = strtol(pFieldStart, &pEnd, 10);
                            if (pEnd == pFieldStart)
                            {
                                return CELLULAR_TASK_FAIL;
                            }
                            sValue = CLAMP(sValue, INT8_MIN, INT8_MAX);
                            pInfo->s8Rsrq = (int8_t)sValue;
                        }
                        else if (u8FieldIndex == 11) // RSRP
                        {
                            sValue = strtol(pFieldStart, &pEnd, 10);
                            if (pEnd == pFieldStart)
                            {
                                return CELLULAR_TASK_FAIL;
                            }
                            sValue = CLAMP(sValue, INT8_MIN, INT8_MAX);
                            pInfo->s8Rsrp = (int8_t)sValue;
                        }
                        else if (u8FieldIndex == 12) // RSSI
                        {
                            sValue = strtol(pFieldStart, &pEnd, 10);
                            if (pEnd == pFieldStart)
                            {
                                return CELLULAR_TASK_FAIL;
                            }
                            sValue = CLAMP(sValue, INT8_MIN, INT8_MAX);
                            pInfo->s8Rssi = (int8_t)sValue;
                        }
                        else if (u8FieldIndex == 13) // RSSNR
                        {
                            sValue = strtol(pFieldStart, &pEnd, 10);
                            if (pEnd == pFieldStart)
                            {
                                return CELLULAR_TASK_FAIL;
                            }
                            sValue = CLAMP(sValue, INT8_MIN, INT8_MAX);
                            pInfo->s8Rssnr = (int8_t)sValue;
                        }

                        if ((*p == '\0') || (*p == '\r') || (*p == '\n'))
                        {
                            break;
                        }

                        u8FieldIndex++;
                        p++;
                        pFieldStart = p;
                        continue;
                    }

                    p++;
                }

                return CELLULAR_TASK_DONE;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

Cellular_Task_Status_t sv_cellular_update_system_time(const char *pHost, float fTimezone)
{
    typedef enum
    {
        NTP_IDLE = 0,
        NTP_WAIT_CONFIG,
        NTP_WAIT_RESULT
    } Ntp_Status_t;

    static Ntp_Status_t eNtpStatus = NTP_IDLE;
    static const char *const pKeywords1[] = {"\r\nOK\r\n"};
    static const char *const pKeywords2[] = {"+CNTP: 0\r\n"};

    if (eNtpStatus == NTP_IDLE)
    {
        if ((fTimezone < -12.0f) || (fTimezone > 14.0f))
        {
            return CELLULAR_TASK_FAIL;
        }

        int16_t s16TimezoneMinutes = (int16_t)(fTimezone * 60.0f);
        if ((s16TimezoneMinutes % 15) != 0)
        {
            return CELLULAR_TASK_FAIL;
        }

        sv_cellular_flush_buffer();
        char au8AtCommand1[64] = {0};
        snprintf(au8AtCommand1, sizeof(au8AtCommand1), "AT+CNTP=\"%s\",%d\r\n", pHost, (int)(fTimezone * 4));
        drv_cellular_send((const uint8_t *)au8AtCommand1, strlen(au8AtCommand1));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords1, ARRAY_SIZE(pKeywords1), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        eNtpStatus = NTP_WAIT_CONFIG;
        return CELLULAR_TASK_WAIT;
    }

    if (eNtpStatus == NTP_WAIT_CONFIG)
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else if (eResult == CELLULAR_WAIT_OK)
        {
            sv_cellular_flush_buffer();

            static const uint8_t au8AtCommand2[] = "AT+CNTP\r\n";
            drv_cellular_send((const uint8_t *)au8AtCommand2, strlen((const char *)au8AtCommand2));

            sv_cellular_wait_for_keywords_start(&sContext, pKeywords2, ARRAY_SIZE(pKeywords2), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);

            eNtpStatus = NTP_WAIT_RESULT;
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            eNtpStatus = NTP_IDLE;
            return CELLULAR_TASK_FAIL;
        }
    }

    if (eNtpStatus == NTP_WAIT_RESULT)
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            eNtpStatus = NTP_IDLE;

            if (eResult == CELLULAR_WAIT_OK)
            {
                return CELLULAR_TASK_DONE;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }

    eNtpStatus = NTP_IDLE;
    return CELLULAR_TASK_FAIL;
}

Cellular_Task_Status_t sv_cellular_get_time(uint8_t *u8Year, uint8_t *u8Month, uint8_t *u8Date, uint8_t *u8Hours, uint8_t *u8Minutes, uint8_t *u8Seconds)
{
    static bool bTaskStarted = false;
    static const char *const pKeywords[] = {"\r\nOK\r\n"};

    if (!bTaskStarted)
    {
        bTaskStarted = true;
        sv_cellular_flush_buffer();

        static const uint8_t au8AtCommand[] = "AT+CCLK?\r\n";
        drv_cellular_send((uint8_t *)au8AtCommand, strlen((const char *)au8AtCommand));

        sv_cellular_wait_for_keywords_start(&sContext, pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL);
        return CELLULAR_TASK_WAIT;
    }
    else
    {
        Cellular_Wait_Result_t eResult = sv_cellular_wait_for_keywords_poll(&sContext);

        if (eResult == CELLULAR_WAIT_PENDING)
        {
            return CELLULAR_TASK_WAIT;
        }
        else
        {
            bTaskStarted = false;

            if (eResult == CELLULAR_WAIT_OK)
            {
                /*
                 * Format: +CCLK: "yy/MM/dd,hh:mm:ss±zz"
                 * zz = Timezone * 4
                 */
                char *p = strstr((char *)au8RxBuf, "+CCLK:");
                if (p)
                {
                    p += 6; // Skip "+CCLK:"
                    while ((*p < '0') || (*p > '9'))
                    {
                        p++;
                    }
                    *u8Year = (uint8_t)strtol(p, &p, 10);
                    p++; // Skip "/"
                    *u8Month = (uint8_t)strtol(p, &p, 10);
                    p++; // Skip "/"
                    *u8Date = (uint8_t)strtol(p, &p, 10);
                    p++; // Skip ","
                    *u8Hours = (uint8_t)strtol(p, &p, 10);
                    p++; // Skip ":"
                    *u8Minutes = (uint8_t)strtol(p, &p, 10);
                    p++; // Skip ":"
                    *u8Seconds = (uint8_t)strtol(p, &p, 10);
                    return CELLULAR_TASK_DONE;
                }

                return CELLULAR_TASK_FAIL;
            }
            else
            {
                return CELLULAR_TASK_FAIL;
            }
        }
    }
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static void sv_cellular_flush_buffer(void)
{
    memset(au8RxBuf, 0, CELLULAR_BUFFER_SIZE);
    u16RxBufSize = 0;
}

static void sv_cellular_wait_for_keywords_start(Cellular_Wait_Context_t *pCtx, const char *const *pKeywords, uint8_t u8KeywordCount, uint32_t u32Timeout)
{
    pCtx->pKeywords = pKeywords;
    pCtx->u8KeywordCount = u8KeywordCount;
    pCtx->u32Timeout = u32Timeout;
    pCtx->u32TimeStart = sys_time_ms();
    pCtx->bActive = true;
}

static Cellular_Wait_Result_t sv_cellular_wait_for_keywords_poll(Cellular_Wait_Context_t *pCtx)
{
    if ((pCtx == NULL) || !pCtx->bActive || (pCtx->pKeywords == NULL) || (pCtx->u8KeywordCount == 0))
    {
        return CELLULAR_WAIT_ERROR;
    }

    if (drv_cellular_receive(au8RxBuf, &u16RxBufSize))
    {
        au8RxBuf[u16RxBufSize] = '\0';

        for (uint8_t i = 0; i < pCtx->u8KeywordCount; i++)
        {
            if (strstr((char *)au8RxBuf, pCtx->pKeywords[i]) != NULL)
            {
                pCtx->bActive = false;
                return CELLULAR_WAIT_OK;
            }
        }
    }

    if (sys_time_ms() - pCtx->u32TimeStart > pCtx->u32Timeout)
    {
        pCtx->bActive = false;
        return CELLULAR_WAIT_TIMEOUT;
    }

    return CELLULAR_WAIT_PENDING;
}