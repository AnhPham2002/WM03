#pragma once

#include "drv_cellular.h"

#define CELLULAR_BUFFER_SIZE UART3_RX_BUFFER_SIZE

#define AT_COMMAND_RESPONSE_TIMEOUT_NORMAL 9000	   // ms
#define AT_COMMAND_RESPONSE_TIMEOUT_NETWORK 120000 // ms
#define HES_RESPONSE_TIMEOUT 10000 // ms

/**
 * @brief Initialize cellular module service.
 */
void sv_cellular_init(void);

/**
 * @brief Turn on cellular module service.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_on(void);

/**
 * @brief Turn off cellular module service.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_off(void);

/**
 * @brief Reset cellular module.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_reset(void);

/**
 * @brief Check cellular module AT command response.
 *
 * @return true if the module responds successfully, otherwise false.
 */
bool sv_cellular_check_at(void);

/**
 * @brief Disable cellular module command echo.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_off_echo(void);

/**
 * @brief Check SIM card status.
 *
 * @return true if the SIM card is ready, otherwise false.
 */
bool sv_cellular_check_sim(void);

/**
 * @brief Check cellular network registration status.
 *
 * @return true if the module is registered, otherwise false.
 */
bool sv_cellular_check_registration_status(void);

/**
 * @brief Start socket service.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_start_socket_service(void);

/**
 * @brief Stop socket service.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_stop_socket_service(void);

/**
 * @brief Get socket service status.
 *
 * @param[out] bStatus Socket service status.
 *
 * @return true if @p bStatus is valid, otherwise false.
 */
bool sv_cellular_check_socket_service(bool *bStatus);

/**
 * @brief Connect to TCP socket.
 *
 * @param[in] pIp   Server IP address.
 * @param[in] pPort Server port.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_connect_tcp_socket(const uint8_t *pIp, uint8_t *pPort);

/**
 * @brief Close TCP socket.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_close_tcp_socket(void);

/**
 * @brief Get TCP socket status.
 *
 * @param[out] bStatus TCP socket status.
 *
 * @return true if @p bStatus is valid, otherwise false.
 */
bool sv_cellular_check_tcp_socket(bool *bStatus);

/**
 * @brief Send data through cellular connection.
 *
 * @param[in] pData   Data buffer.
 * @param[in] u16Size Data size.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_send_data(const uint8_t *pData, uint16_t u16Size);

/**
 * @brief Receive data from cellular connection.
 *
 * @param[out] pData      Receive buffer.
 * @param[in,out] u16Size Data size.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_receive_data(uint8_t *pData, uint16_t *u16Size);

/**
 * @brief Get SIM card CCID.
 *
 * @param[out] pCcid CCID buffer.
 * @param[out] u8Len CCID length.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_get_ccid(uint8_t *pCcid, uint8_t *u8Len);

/**
 * @brief Get cellular signal quality.
 *
 * @param[out] s8Rssi RSSI value.
 * @param[out] s8Rsrp RSRP value.
 * @param[out] s8Rsrq RSRQ value.
 * @param[out] s8Rssnr RSSNR value.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_check_signal_quality(int8_t *s8Rssi, int8_t *s8Rsrp, int8_t *s8Rsrq, int8_t *s8Rssnr);

/**
 * @brief Update system time from NTP server.
 *
 * @param[in] pHost     NTP server host.
 * @param[in] fTimezone Timezone offset.
 *
 * @return true if successful, otherwise false.
 */
bool sv_cellular_update_system_time(const char *pHost, float fTimezone);

/**
 * @brief Get cellular module date and time.
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
bool sv_cellular_get_time(uint8_t *u8Year, uint8_t *u8Month, uint8_t *u8Date, uint8_t *u8Hours, uint8_t *u8Minutes, uint8_t *u8Seconds);