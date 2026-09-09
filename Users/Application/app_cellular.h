#pragma once

#include "sv_cellular.h"

#include "app_storage.h"

#define CELLULAR_RETRY 3

#define CELLULAR_GET_INFO_IDLE_TIMEOUT 15000
#define CELLULAR_COMMUNICATE_IDLE_TIMEOUT HES_RESPONSE_TIMEOUT

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

/**
 * @brief Initialize cellular application.
 */
void app_cellular_init(void);

/**
 * @brief Start cellular application.
 */
void app_cellular_start(void);

/**
 * @brief Execute cellular communication process.
 */
void app_cellular_execute(void);

/**
 * @brief Activate cellular data push.
 */
void app_cellular_push_activate(void);

/**
 * @brief Get SIM card CCID from LTE module.
 *
 * Starts the LTE module if it is idle and waits until the module is ready.
 *
 * @param[out] pCcid      CCID output buffer.
 * @param[out] u8CcidLen  CCID length.
 *
 * @retval TASK_STATUS_RUNNING LTE module is not ready yet.
 * @retval TASK_STATUS_SUCCESS CCID was read successfully.
 * @retval TASK_STATUS_FAILED  CCID read failed.
 */
Task_Status_t app_cellular_get_ccid(uint8_t *pCcid, uint8_t *u8CcidLen);

/**
 * @brief Get LTE signal quality information.
 *
 * Starts the LTE module if it is idle and waits until the module is ready.
 *
 * @param[out] s8Rssi RSSI value.
 * @param[out] s8Rsrp RSRP value.
 * @param[out] s8Rsrq RSRQ value.
 * @param[out] s8Rssnr RSSNR value.
 *
 * @retval TASK_STATUS_RUNNING LTE module is not ready yet.
 * @retval TASK_STATUS_SUCCESS Signal quality was read successfully.
 * @retval TASK_STATUS_FAILED  Signal quality read failed.
 */
Task_Status_t app_cellular_get_signal_quality(int8_t *s8Rssi, int8_t *s8Rsrp, int8_t *s8Rsrq, int8_t *s8Rssnr);

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
