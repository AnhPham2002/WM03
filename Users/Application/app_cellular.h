#pragma once

#include "sv_cellular.h"

#include "app_storage.h"

#define CELLULAR_RETRY 3

#define CELLULAR_GET_INFO_TIMEOUT 15000
#define CELLULAR_GET_INFO_IDLE_TIMEOUT 15000

#define NTP_SERVER_PRIMARY "time.google.com"
#define NTP_SERVER_BACKUP_1 "time.windows.com"
#define NTP_SERVER_BACKUP_2 "vn.pool.ntp.org"

typedef enum
{
	CELLULAR_STEP_IDLE = 0,
	CELLULAR_STEP_ON,
	CELLULAR_STEP_WAIT_AT_READY,
	CELLULAR_STEP_TURN_OFF_ECHO,
	CELLULAR_STEP_CHECK_SIM,
	CELLULAR_STEP_CHECK_REGISTRATION_STATUS,
	CELLULAR_STEP_START_SOCKET_SERVICE,
	CELLULAR_STEP_CHECK_SOCKET_SERVICE,
	CELLULAR_STEP_SYNC_DATE_TIME,
	CELLULAR_STEP_GET_DATE_TIME,
	CELLULAR_STEP_CONNECT_TCP_SOCKET,
	CELLULAR_STEP_CHECK_TCP_SOCKET,
	CELLULAR_STEP_COMMUNICATE,
	CELLULAR_STEP_CLOSE_TCP_SOCKET,
	CELLULAR_STEP_STOP_SOCKET_SERVICE,
	CELLULAR_STEP_RESET,
	CELLULAR_STEP_OFF
} Cellular_Step_t;

typedef enum
{
    CELLULAR_ERROR_NONE = 0,
    CELLULAR_ERROR_POWER_ON,
    CELLULAR_ERROR_AT_READY,
    CELLULAR_ERROR_SIM_NOT_READY,
    CELLULAR_ERROR_NETWORK_NOT_REGISTERED,
    CELLULAR_ERROR_SOCKET_SERVICE_NOT_READY,
    CELLULAR_ERROR_DATE_TIME_SYNC,
    CELLULAR_ERROR_GET_DATE_TIME,
    CELLULAR_ERROR_TCP_CONNECT
} Cellular_Error_t;

/**
 * @brief Initialize cellular application.
 */
void app_cellular_init(void);

/**
 * @brief Start cellular application.
 */
void app_cellular_start(void);

/**
 * @brief Stop cellular application.
 */
void app_cellular_stop(void);

/**
 * @brief Execute cellular communication process.
 */
void app_cellular_execute(void);

/**
 * @brief Activate cellular data push.
 */
void app_cellular_push_activate(void);

/**
 * @brief Get SIM card ICCID information.
 *
 * Copies the cached ICCID information if valid; otherwise, clears the output.
 *
 * @param[out] pInfo ICCID information.
 *
 * @return true if the ICCID information is valid, otherwise false.
 */
bool app_cellular_get_ccid(Ccid_Info_t *pInfo);

/**
 * @brief Get cellular signal quality information.
 *
 * Copies the cached signal information if valid; otherwise, clears the output.
 *
 * @param[out] pInfo Cellular signal information.
 *
 * @return true if the signal information is valid, otherwise false.
 */
bool app_cellular_get_signal_quality(Signal_Info_t *pInfo);

/**
 * @brief Send data through LTE TCP socket.
 *
 * Sends data only when the LTE module is connected to HES.
 * Resets the LTE module if data transmission fails.
 *
 * @param[in] pData  Transmit data buffer.
 * @param[in] u16Len Data length in bytes.
 *
 * @return true if data is sent successfully, otherwise false.
 */
bool app_cellular_send_data(const uint8_t *pData, uint16_t u16Len);

/**
 * @brief Receive data through LTE connection.
 *
 * Receives data only when the LTE module is connected.
 *
 * @param[out] pData  Receive data buffer.
 * @param[out] u16Len Received data length.
 *
 * @return true if data is received successfully, otherwise false.
 */
bool app_cellular_receive_data(uint8_t *pData, uint16_t *u16Len);

/**
 * @brief Get cellular connection status.
 *
 * @return true if the cellular connection is active, otherwise false.
 */
bool app_cellular_get_connection_status(void);

/**
 * @brief Get and clear the current cellular error.
 *
 * @param[out] pErr Current cellular error.
 *
 * @return true if the cellular cycle is finished, otherwise false.
 */
bool app_cellular_get_error(Cellular_Error_t *pErr);
