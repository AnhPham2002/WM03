#pragma once

#include "sys_common.h"
#include "aes128.h"

#define ALIGN16(x) (((x) + 15) & ~15)

#define PROTOCOL_KEY {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F}

#define PROTOCOL_START_OF_FRAME 0x5A
#define PROTOCOL_MODULE_TYPE 0x03
#define PROTOCOL_MODULE_SERIAL_COMMON 0x00

typedef struct __attribute__((packed))
{
    uint8_t u8StartOfFrame;
    uint8_t u8ModuleType;
    uint64_t u64ModuleSerial;
    uint8_t u8CmdCode;
    uint8_t u8IdCode;
    uint16_t u16CiphertextLen;
} Protocol_Header_t;

typedef enum
{
    PROTOCOL_ERR_SUCCESS = 0x00,
    PROTOCOL_ERR_CMD_INVALID = 0x01,
    PROTOCOL_ERR_CRC16_INVALID = 0x02,
    PROTOCOL_ERR_FRAME_INVALID = 0x03,
    PROTOCOL_ERR_FW_NO_SPACE = 0x04,
    PROTOCOL_ERR_FW_CRC32_FAILED = 0x05,
    PROTOCOL_ERR_FW_INCOMPLETE = 0x06,
    PROTOCOL_ERR_FLASH_ERASE_FAILED = 0x07,
    PROTOCOL_ERR_FLASH_WRITE_FAILED = 0x08,
    PROTOCOL_ERR_PASSWORD_INCORRECT = 0x10,
    PROTOCOL_ERR_ACCESS_DENIED = 0x11,
    PROTOCOL_ERR_MODULE_SERIAL_INVALID = 0xFD,
    PROTOCOL_ERR_NULL_POINTER = 0xFE,
    PROTOCOL_ERR_UNKNOW = 0xFF
} Protocol_Err_Code_t;

/**
 * @brief Pack data into a protocol frame.
 *
 * Builds a protocol frame containing the module information, command,
 * payload and CRC16. The payload can optionally be encrypted using AES-128.
 *
 * @param[in]     bEncrypt       Enable AES-128 encryption.
 * @param[in]     u64Serial      Module serial number.
 * @param[in]     u8Cmd          Command code.
 * @param[in]     u8Id           ID code.
 * @param[in]     pPayload       Payload data.
 * @param[in]     u16PayloadLen  Payload size in bytes.
 * @param[out]    pFrame         Output frame buffer.
 * @param[out]    u16FrameLen    Output frame size in bytes.
 *
 * @return true if the frame is packed successfully, otherwise false.
 */
bool drv_protocol_pack(bool bEncrypt, uint64_t u64Serial, uint8_t u8Cmd, uint8_t u8Id, const uint8_t *pPayload, uint16_t u16PayloadLen, uint8_t *pFrame, uint16_t *u16FrameLen);

/**
 * @brief Unpack and validate a protocol frame.
 *
 * Verifies the frame header and CRC16, decrypts the payload if required,
 * and extracts the payload data.
 *
 * @param[in]  pFrame         Input protocol frame.
 * @param[in]  u16FrameLen    Frame size in bytes.
 * @param[out] u64Serial      Module serial number.
 * @param[out] u8Cmd          Command code.
 * @param[out] u8Id           ID code.
 * @param[out] pPayload       Output payload buffer.
 * @param[out] u16PayloadLen  Output payload size in bytes.
 *
 * @return Protocol error code.
 */
Protocol_Err_Code_t drv_protocol_unpack(const uint8_t *pFrame, uint16_t u16FrameLen, uint64_t *u64Serial, uint8_t *u8Cmd, uint8_t *u8Id, uint8_t *pPayload, uint16_t *u16PayloadLen);
