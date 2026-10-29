#pragma once

#include "drv_cellular.h"

#define CELLULAR_BUFFER_SIZE UART3_RX_BUFFER_SIZE

#define MAX_SIM_CCID_SIZE 25

#define AT_COMMAND_RESPONSE_TIMEOUT_NORMAL 9000    // ms
#define AT_COMMAND_RESPONSE_TIMEOUT_NETWORK 120000 // ms
#define HES_RESPONSE_TIMEOUT 5000                  // ms

typedef enum
{
    CELLULAR_WAIT_PENDING = 0,
    CELLULAR_WAIT_OK,
    CELLULAR_WAIT_TIMEOUT,
    CELLULAR_WAIT_ERROR
} Cellular_Wait_Result_t;

typedef enum
{
    CELLULAR_TASK_IDLE = 0,
    CELLULAR_TASK_SEND,
    CELLULAR_TASK_WAIT,
    CELLULAR_TASK_DONE,
    CELLULAR_TASK_FAIL
} Cellular_Task_Status_t;

typedef struct
{
    const char *const *pKeywords;
    uint8_t u8KeywordCount;
    uint32_t u32Timeout;
    uint32_t u32TimeStart;
    bool bActive;
} Cellular_Wait_Context_t;

typedef struct
{
    uint8_t au8Ccid[MAX_SIM_CCID_SIZE];
    uint8_t u8CcidLen;
} Ccid_Info_t;

typedef struct
{
    int8_t s8Rssi;
    int8_t s8Rsrp;
    int8_t s8Rsrq;
    int8_t s8Rssnr;
} Signal_Info_t;

/**
 * @brief Initialize cellular module service.
 */
void sv_cellular_init(void);

/**
 * @brief Power on the cellular module.
 *
 * Executes the hardware power-on sequence and waits for the UART interface
 * to become ready.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_on(void);

/**
 * @brief Power off the cellular module.
 *
 * Sends the AT+CPOF command and waits for the shutdown response.
 * Performs a hardware power-off if the command fails or times out.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_off(void);

/**
 * @brief Reset the cellular module.
 *
 * Sends the AT+CRESET command and waits for the reset response.
 * Performs a hardware reset if the command fails or times out.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_reset(void);

/**
 * @brief Check AT command communication with the cellular module.
 *
 * Sends the AT command and waits for an OK response.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_check_at(void);

/**
 * @brief Disable command echo on the cellular module.
 *
 * Sends the ATE0 command and waits for an OK response.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_off_echo(void);

/**
 * @brief Check SIM card readiness.
 *
 * Sends the AT+CPIN? command and waits for the SIM READY response.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_check_sim(void);

/**
 * @brief Check cellular network registration status.
 *
 * Sends the AT+CEREG? command and waits for home network or roaming
 * registration confirmation.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_check_registration_status(void);

/**
 * @brief Start the cellular network socket service.
 *
 * Sends the AT+NETOPEN command and waits for confirmation that the
 * network service is open or already opened.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_start_socket_service(void);

/**
 * @brief Stop the cellular network socket service.
 *
 * Sends the AT+NETCLOSE command and waits for confirmation that the
 * service is closed or already closed.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_stop_socket_service(void);

/**
 * @brief Check the cellular network socket service status.
 *
 * Sends the AT+NETOPEN? command and parses the service status from the response.
 *
 * @param[out] bStatus Socket service status (true if open, otherwise false).
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_check_socket_service(bool *bStatus);

/**
 * @brief Establish a TCP socket connection.
 *
 * Sends the AT+CIPOPEN command to connect to the specified IP address
 * and port.
 *
 * @param[in] pIp   Server IP address as a 15-character string.
 * @param[in] pPort Server port string.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_connect_tcp_socket(const uint8_t *pIp, uint8_t *pPort);

/**
 * @brief Close the TCP socket connection.
 *
 * Sends the AT+CIPCLOSE=0 command and waits for confirmation that
 * the socket is closed or already closed.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_close_tcp_socket(void);

/**
 * @brief Check the TCP socket connection status.
 *
 * Sends the AT+CIPCLOSE? command and parses the socket connection status
 * from the response.
 *
 * @param[out] bStatus Socket connection status (true if connected, otherwise false).
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_check_tcp_socket(bool *bStatus);

/**
 * @brief Send data through the TCP socket.
 *
 * Requests a send prompt, transmits the payload, and waits for confirmation
 * that the requested data length was sent successfully.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size in bytes.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_send_data(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from the TCP socket.
 *
 * Retrieves incoming data from the receive buffer or waits for an +IPD
 * notification, then extracts the payload and its length.
 *
 * @param[out] pData   Output data buffer.
 * @param[out] u16Size Received data size in bytes.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_receive_data(uint8_t *pData, uint16_t *u16Size);

/**
 * @brief Retrieve the SIM card ICCID.
 *
 * Sends the AT+CICCID command and parses the ICCID from the response.
 *
 * @param[out] pInfo SIM card ICCID information.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_get_ccid(Ccid_Info_t *pInfo);

/**
 * @brief Retrieve cellular signal quality information.
 *
 * Sends the AT+CPSI? command and parses RSRQ, RSRP, RSSI, and RSSNR
 * from the response.
 *
 * @param[out] pInfo Cellular signal quality information.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_check_signal_quality(Signal_Info_t *pInfo);

/**
 * @brief Synchronize system time with an NTP server.
 *
 * Configures the NTP server and timezone, then requests time synchronization.
 *
 * @param[in] pHost      NTP server hostname.
 * @param[in] fTimezone  Timezone offset in hours, from UTC-12 to UTC+14,
 *                       in 15-minute increments.
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_update_system_time(const char *pHost, float fTimezone);

/**
 * @brief Retrieve date and time from the cellular module.
 *
 * Sends the AT+CCLK? command and parses the year, month, date, hours,
 * minutes, and seconds from the response.
 *
 * @param[out] u8Year    Year (two-digit format).
 * @param[out] u8Month   Month (1–12).
 * @param[out] u8Date    Day of the month (1–31).
 * @param[out] u8Hours   Hour (0–23).
 * @param[out] u8Minutes Minute (0–59).
 * @param[out] u8Seconds Second (0–59).
 *
 * @return Current task status.
 */
Cellular_Task_Status_t sv_cellular_get_time(uint8_t *u8Year, uint8_t *u8Month, uint8_t *u8Date, uint8_t *u8Hours, uint8_t *u8Minutes, uint8_t *u8Seconds);