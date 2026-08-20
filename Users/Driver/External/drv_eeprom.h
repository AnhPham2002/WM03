#pragma once

#include "drv_spi.h"
#include "drv_gpio.h"
#include "sys_common.h"

#define SPI_CS_EE_PIN GPIOA, GPIO_PIN_4

/* M95512 EEPROM Info*/
#define EE_CMD_SIZE 1
#define EE_ADDR_SIZE 2
#define EE_PAGE_SIZE 128
#define EE_TOTAL_SIZE 65536 // 64 kB of EEPROM
#define EE_WAIT_TIMEOUT 10  // Timeout 10 ms (EEPROM Write Cycle Max: 5 ms)

/* M95512 SPI Command Definitions */
#define EE_CMD_WREN (uint8_t)0x06   // Write enable
#define EE_CMD_WRDI (uint8_t)0x04   // Write disable
#define EE_CMD_RDSR (uint8_t)0x05   // Read Status register
#define EE_CMD_WRSR (uint8_t)0x01   // Write Status register
#define EE_CMD_READ (uint8_t)0x03   // Read from Memory array
#define EE_CMD_WRITE (uint8_t)0x02  // Write to Memory array
#define EE_DUMMY_BYTE (uint8_t)0xFF // Dummy byte used to generate SPI clock for data reception

/* M95512 Status Register */
// Bit mask
#define EE_SR_WIP_POS 0 // Write in progress bit position
#define EE_SR_WIP_MASK (1U << EE_SR_WIP_POS)
#define EE_SR_WEL_POS 1 // Write enable latch bit position
#define EE_SR_WEL_MASK (1U << EE_SR_WEL_POS)
#define EE_SR_BP_POS 2 // Block protect bits position
#define EE_SR_BP_MASK (3U << EE_SR_BP_POS)
#define EE_SR_SRWD_POS 7 // Status register Write protect position
#define EE_SR_SRWD_MASK (1U << EE_SR_SRWD_POS)
// Status register Write protect bit value
#define EE_SR_SRWD_DIS 0x00
#define EE_SR_SRWD_EN 0x01
// Block protect bits value
#define EE_SR_BP_NONE 0x00 // Protected array addresses: None
#define EE_SR_BP_UQ 0x01   // Protected array addresses: C000h - FFFFh (Upper quarter)
#define EE_SR_BP_UH 0x02   // Protected array addresses: 8000h - FFFFh (Upper half)
#define EE_SR_BP_ALL 0x03  // Protected array addresses: 0000h - FFFFh (Whole memory)
// Write enable latch bit value
#define EE_SR_WEL_RESET 0x00
#define EE_SR_WEL_SET 0x01
// Write in progress bit value
#define EE_SR_WIP_READY 0x00
#define EE_SR_WIP_BUSY 0x01

/**
 * @brief          Initialize the EEPROM driver
 */
void drv_eeprom_init(void);

/**
 * @brief  Read data from EEPROM.
 *
 * This function reads the specified number of bytes from the EEPROM
 * starting at the given address. Before reading, it checks whether
 * the EEPROM is busy and waits for the current write cycle to complete.
 *
 * @param[in]  u16Address  Start address in EEPROM.
 * @param[out] pData       Pointer to receive buffer.
 * @param[in]  u16Size     Number of bytes to read.
 *
 * @retval true   Data read successfully.
 * @retval false  Invalid parameter, address out of range,
 *                or EEPROM remained busy after timeout.
 */
bool drv_eeprom_read_data(uint16_t u16Address, uint8_t *pData, uint16_t u16Size);

/**
 * @brief  Write data to EEPROM.
 *
 * This function writes the specified number of bytes to the EEPROM
 * starting at the given address. The data is automatically split into
 * page-sized write operations to prevent crossing EEPROM page boundaries.
 * Before each page write, the write-enable command is issued and the
 * function waits for the internal write cycle to complete.
 *
 * @param[in] u16Address  Start address in EEPROM.
 * @param[in] pData       Pointer to transmit buffer.
 * @param[in] u16Size     Number of bytes to write.
 *
 * @retval true   Data written successfully.
 * @retval false  Invalid parameter, address out of range,
 *                or EEPROM remained busy after timeout.
 */
bool drv_eeprom_write_data(uint16_t u16Address, const uint8_t *pData, uint16_t u16Size);