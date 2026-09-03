#pragma once

#include "drv_4g.h"

#define MODULE_4G_BUFFER_SIZE UART3_RX_BUFFER_SIZE

#define AT_COMMAND_RESPONSE_TIMEOUT_NORMAL 9000	   // ms
#define AT_COMMAND_RESPONSE_TIMEOUT_NETWORK 120000 // ms
#define HES_RESPONSE_TIMEOUT 10000 // ms

/**
 * @brief Initialize 4G service.
 */
void sv_4g_init(void);

/**
 * @brief Turn on 4G service.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_on(void);

/**
 * @brief Turn off 4G service.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_off(void);

/**
 * @brief Reset 4G module.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_reset(void);

/**
 * @brief Check 4G module AT command response.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_check_at(void);

/**
 * @brief Disable 4G module command echo.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_off_echo(void);

/**
 * @brief Check SIM card status.
 *
 * @return true if SIM card is ready, otherwise false.
 */
bool sv_4g_check_sim(void);

/**
 * @brief Check 4G network registration status.
 *
 * @return true if registered, otherwise false.
 */
bool sv_4g_check_registration_status(void);

/**
 * @brief Start 4G socket service.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_start_socket_service(void);

/**
 * @brief Stop 4G socket service.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_stop_socket_service(void);

/**
 * @brief Get 4G socket service status.
 *
 * @param[out] bStatus Socket service status.
 *
 * @return true if @p bStatus is valid, otherwise false.
 */
bool sv_4g_check_socket_service(bool *bStatus);

/**
 * @brief Connect to TCP socket.
 *
 * @param[in] pIp   Server IP address.
 * @param[in] pPort Server port.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_connect_tcp_socket(const uint8_t *pIp, uint8_t *pPort);

/**
 * @brief Close TCP socket.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_close_tcp_socket(void);

/**
 * @brief Get TCP socket status.
 *
 * @param[out] bStatus TCP socket status.
 *
 * @return true if @p bStatus is valid, otherwise false.
 */
bool sv_4g_check_tcp_socket(bool *bStatus);

/**
 * @brief Send data through 4G connection.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_send_data(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from 4G connection.
 *
 * @param[out] pData     Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_receive_data(uint8_t *pData, uint16_t *u16Size);

/**
 * @brief Get SIM card CCID.
 *
 * @param[out] pCcid CCID buffer.
 * @param[out] u8Len CCID length.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_get_ccid(uint8_t *pCcid, uint8_t *u8Len);

/**
 * @brief Get 4G signal quality.
 *
 * @param[out] s8Rssi RSSI value.
 * @param[out] s8Rsrp RSRP value.
 * @param[out] s8Rsrq RSRQ value.
 * @param[out] s8Rssnr RSSNR value.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_check_signal_quality(int8_t *s8Rssi, int8_t *s8Rsrp, int8_t *s8Rsrq, int8_t *s8Rssnr);

/**
 * @brief Update system time from NTP server.
 *
 * @param[in] pHost     NTP server host.
 * @param[in] fTimezone Timezone offset.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_update_system_time(const char *pHost, float fTimezone);

/**
 * @brief Get 4G module date and time.
 *
 * @param[out] u8Year    Year.
 * @param[out] u8Month   Month.
 * @param[out] u8Date    Date.
 * @param[out] u8Hours   Hour.
 * @param[out] u8Minutes Minute.
 * @param[out] u8Seconds Second.
 *
 * @return true if successful, otherwise false.
 */
bool sv_4g_get_time(uint8_t *u8Year, uint8_t *u8Month, uint8_t *u8Date, uint8_t *u8Hours, uint8_t *u8Minutes, uint8_t *u8Seconds);