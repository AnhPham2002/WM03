#include "sv_4g.h"
#include "sys_common.h"

static uint8_t au8RxBuf[MODULE_4G_BUFFER_SIZE];
static uint16_t u16RxBufSize;

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Clear 4G receive buffer.
 */
static void sv_4g_flush_buffer(void);

/**
 * @brief Wait for one of the specified keywords from 4G module.
 *
 * @param[in] pKeywords     Keyword list.
 * @param[in] u8KeywordCount Number of keywords.
 * @param[in] u32Timeout    Timeout in milliseconds.
 *
 * @return true if a keyword is received, otherwise false.
 */
static bool sv_4g_wait_for_keywords(const char *const *pKeywords, uint8_t u8KeywordCount, uint32_t u32Timeout);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void sv_4g_init(void)
{
    drv_4g_init();
}

bool sv_4g_on(void)
{
    sv_4g_flush_buffer();
    drv_4g_pwr_on();
    sys_delay_ms(50); // Wait for charge capacitor
    drv_4g_turn_on();

    const char *pKeywords[] = {"\r\nQCRDY\r\n"};
    if (!sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        drv_4g_pwr_off();
        return false;
    }

    return true;
}

bool sv_4g_off(void)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+CPOF\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\nOK\r\n", "POWERED DOWN"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        /* Soft off */
        sys_delay_ms(1000); // Delay for safety shutdown
        drv_4g_pwr_off();
    }
    else
    {
        /* Hard off */
        drv_4g_turn_off();
        HAL_Delay(1000); // Delay for safety shutdown
        drv_4g_pwr_off();
    }

    return true;
}

bool sv_4g_reset(void)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+CRESET\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\nOK\r\n", "\r\nQCRDY\r\n"};
    if (!sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        /* Hard reset */
        drv_4g_reset();
    }

    return true;
}

bool sv_4g_check_at(void)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\nOK\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        return true;
    }

    return false;
}

bool sv_4g_off_echo(void)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "ATE0\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\nOK\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        return true;
    }

    return false;
}

bool sv_4g_check_sim(void)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+CPIN?\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\n+CPIN: READY\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        return true;
    }

    return false;
}

bool sv_4g_check_registration_status(void)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+CEREG?\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    /*
     * 1 registered, home network
     * 5 registered, roaming
     */
    const char *pKeywords[] = {"\r\n+CEREG: 0,1\r\n", "\r\n+CEREG: 0,5\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        return true;
    }

    return false;
}

bool sv_4g_start_socket_service(void)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+NETOPEN\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\n+NETOPEN: 0\r\n", "\r\n+IP ERROR: Network is already opened\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        return true;
    }

    return false;
}

bool sv_4g_stop_socket_service(void)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+NETCLOSE\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\n+NETCLOSE: 0\r\n", "\r\n+NETCLOSE: 2\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        return true;
    }

    return false;
}

bool sv_4g_check_socket_service(bool *bStatus)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+NETOPEN?\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\nOK\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        char *p = strstr((char *)au8RxBuf, "+NETOPEN:");
        if (p)
        {
            p += 9; // Skip "+NETOPEN:"
            while (*p == ' ')
                p++;

            if ((*p == '0') || (*p == '1'))
            {
                *bStatus = (bool)(*p - '0');
                return true;
            }
        }
    }

    return false;
}

bool sv_4g_connect_tcp_socket(const uint8_t *pIp, uint8_t *pPort)
{
    char pIpStr[16];
    memcpy(pIpStr, pIp, 15);
    pIpStr[15] = '\0';

    sv_4g_flush_buffer();
    char au8AtCommand[64] = {0};
    snprintf(au8AtCommand, sizeof(au8AtCommand), "AT+CIPOPEN=0,\"TCP\",\"%s\",%s\r\n", pIpStr, pPort);
    drv_4g_send((const uint8_t *)au8AtCommand, strlen(au8AtCommand));

    const char *pKeywords[] = {"\r\nOK\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        return true;
    }

    return false;
}

bool sv_4g_close_tcp_socket(void)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+CIPCLOSE=0\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\n+CIPCLOSE: 0,0\r\n", "\r\n+CIPCLOSE: 0,4\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        return true;
    }

    return false;
}

bool sv_4g_check_tcp_socket(bool *bStatus)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+CIPCLOSE?\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\nOK\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        char *p = strstr((char *)au8RxBuf, "+CIPCLOSE:");
        if (p)
        {
            p += 10; // Skip "+CIPCLOSE:"
            while (*p == ' ')
                p++;

            if ((*p == '0') || (*p == '1'))
            {
                *bStatus = (bool)(*p - '0');
                return true;
            }
        }
    }

    return false;
}

bool sv_4g_send_data(const uint8_t *pData, uint16_t u16Size)
{
    if (pData == NULL)
    {
        return false;
    }

    sv_4g_flush_buffer();
    char au8AtCommand[32] = {0};
    snprintf(au8AtCommand, sizeof(au8AtCommand), "AT+CIPSEND=0,%u\r\n", (unsigned int)u16Size);
    drv_4g_send((const uint8_t *)au8AtCommand, strlen(au8AtCommand));

    const char *pKeywords1[] = {">"};
    if (sv_4g_wait_for_keywords(pKeywords1, ARRAY_SIZE(pKeywords1), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        sv_4g_flush_buffer();
        drv_4g_send(pData, u16Size);
        drv_4g_send((const uint8_t *)0x1A, 1); // Send CTRL+Z

        const char *pKeywords2[] = {"+CIPSEND:"}; // Syntax response: +CIPSEND: <link_num>,<reqSendLength>,<cnfSendLength>
        if (sv_4g_wait_for_keywords(pKeywords2, ARRAY_SIZE(pKeywords2), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
        {
            uint16_t u16RequestedSendLen, u16ConfirmedSendLen;

            char *p = strchr((char *)au8RxBuf, ','); // Find the first ','
            p++;                                     // Skip ','
            u16RequestedSendLen = atoi(p);

            if (u16Size != u16RequestedSendLen)
            {
                return false;
            }

            p = strchr(p, ','); // Find the second ','
            p++;                // Skip ','
            u16ConfirmedSendLen = atoi(p);

            if (u16RequestedSendLen == u16ConfirmedSendLen)
            {
                return true;
            }
        }
    }

    return false;
}

bool sv_4g_receive_data(uint8_t *pData, uint16_t *u16Size)
{
    if ((pData == NULL) || (u16Size == NULL))
    {
        return false;
    }

    /*
     * RECV FROM:<IP ADDRESS>:<PORT><CR><LF>
     * +IPD<data length><CR><LF>
     * <data>
     */
    char *p = strstr((char *)au8RxBuf, "+IPD");
    if (p == NULL)
    {
        const char *pKeywords[] = {"\r\n+IPD"};
        if (!sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), HES_RESPONSE_TIMEOUT))
        {
            return false;
        }
        p = strstr((char *)au8RxBuf, "+IPD");
        if (p == NULL)
        {
            return false;
        }
    }

    p += 4; // Skip "+IPD"
    *u16Size = atoi(p);
    p = strstr(p, "\r\n");
    if (p == NULL)
    {
        return false;
    }
    p += 2; // Skip "\r\n"
    memcpy(pData, p, *u16Size);

    sv_4g_flush_buffer();

    return true;
}

bool sv_4g_get_ccid(uint8_t *pCcid, uint8_t *u8Len)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+CICCID\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\nOK\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        char *p = strstr((char *)au8RxBuf, "+ICCID:");
        if (p)
        {
            p += 7; // Skip "+ICCID:"
            while (*p == ' ')
                p++;

            char *pEnd = strchr(p, '\r');
            if (pEnd)
            {
                *u8Len = pEnd - p;
                memcpy(pCcid, p, *u8Len);
                return true;
            }
        }
    }

    return false;
}

bool sv_4g_check_signal_quality(int8_t *s8Rssi, int8_t *s8Rsrp, int8_t *s8Rsrq, int8_t *s8Rssnr)
{
    sv_4g_flush_buffer();
    uint8_t au8AtCommand[] = "AT+CPSI?\r\n";
    drv_4g_send(au8AtCommand, strlen((const char *)au8AtCommand));

    const char *pKeywords[] = {"\r\nOK\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        const char *p;
        const char *pFieldStart;
        char *pEnd;
        uint8_t u8FieldIndex = 0;
        int sValue;

        p = strchr((char *)au8RxBuf, ':');
        if (p == NULL)
        {
            return false;
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
                        return false;
                    }
                    sValue = CLAMP(sValue, INT8_MIN, INT8_MAX);
                    *s8Rsrq = (int8_t)sValue;
                }
                else if (u8FieldIndex == 11) // RSRP
                {
                    sValue = strtol(pFieldStart, &pEnd, 10);
                    if (pEnd == pFieldStart)
                    {
                        return false;
                    }
                    sValue = CLAMP(sValue, INT8_MIN, INT8_MAX);
                    *s8Rsrp = (int8_t)sValue;
                }
                else if (u8FieldIndex == 12) // RSSI
                {
                    sValue = strtol(pFieldStart, &pEnd, 10);
                    if (pEnd == pFieldStart)
                    {
                        return false;
                    }
                    sValue = CLAMP(sValue, INT8_MIN, INT8_MAX);
                    *s8Rssi = (int8_t)sValue;
                }
                else if (u8FieldIndex == 13) // RSSNR
                {
                    sValue = strtol(pFieldStart, &pEnd, 10);
                    if (pEnd == pFieldStart)
                    {
                        return false;
                    }
                    sValue = CLAMP(sValue, INT8_MIN, INT8_MAX);
                    *s8Rssnr = (int8_t)sValue;
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

        return true;
    }

    return false;
}

bool sv_4g_update_system_time(const char *pHost, float fTimezone)
{
    if ((fTimezone < -12.0f) || (fTimezone > 14.0f))
    {
        return false;
    }

    int16_t s16TimezoneMinutes = (int16_t)(fTimezone * 60.0f);
    if ((s16TimezoneMinutes % 15) != 0)
    {
        return false;
    }

    sv_4g_flush_buffer();
    char au8AtCommand1[64] = {0};
    snprintf(au8AtCommand1, sizeof(au8AtCommand1), "AT+CNTP=\"%s\",%d\r\n", pHost, (int)(fTimezone * 4));
    drv_4g_send((const uint8_t *)au8AtCommand1, strlen(au8AtCommand1));

    const char *pKeywords1[] = {"\r\nOK\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords1, ARRAY_SIZE(pKeywords1), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
    {
        sv_4g_flush_buffer();
        char au8AtCommand2[] = "AT+CNTP\r\n";
        drv_4g_send((const uint8_t *)au8AtCommand2, strlen(au8AtCommand2));

        const char *pKeywords2[] = {"+CNTP: 0\r\n"};
        if (sv_4g_wait_for_keywords(pKeywords2, ARRAY_SIZE(pKeywords2), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
        {
            return true;
        }
    }

    return false;
}

bool sv_4g_get_time(uint8_t *u8Year, uint8_t *u8Month, uint8_t *u8Date, uint8_t *u8Hours, uint8_t *u8Minutes, uint8_t *u8Seconds)
{
    sv_4g_flush_buffer();
    char au8AtCommand[] = "AT+CCLK?\r\n";
    drv_4g_send((const uint8_t *)au8AtCommand, strlen(au8AtCommand));

    const char *pKeywords[] = {"\r\nOK\r\n"};
    if (sv_4g_wait_for_keywords(pKeywords, ARRAY_SIZE(pKeywords), AT_COMMAND_RESPONSE_TIMEOUT_NORMAL))
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
                p++;
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
            return true;
        }
    }

    return false;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static void sv_4g_flush_buffer(void)
{
    memset(au8RxBuf, 0, MODULE_4G_BUFFER_SIZE);
    u16RxBufSize = 0;
}

static bool sv_4g_wait_for_keywords(const char *const *pKeywords, uint8_t u8KeywordCount, uint32_t u32Timeout)
{
    static uint32_t u32TimeStart;

    if ((pKeywords == NULL) || (u8KeywordCount == 0))
    {
        return false;
    }

    u32TimeStart = sys_time_ms();
    while (1)
    {
        if (drv_4g_receive(au8RxBuf, &u16RxBufSize))
        {
            for (uint8_t i = 0; i < u8KeywordCount; i++)
            {
                if (strstr((char *)au8RxBuf, pKeywords[i]))
                {
                    return true;
                }
            }

            return false;
        }

        if (sys_time_ms() - u32TimeStart > u32Timeout)
        {
            return false;
        }
    }
}