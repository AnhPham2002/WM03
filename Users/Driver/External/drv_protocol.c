#include "drv_protocol.h"

static const uint8_t au8Key[16] = PROTOCOL_KEY;

bool drv_protocol_pack(bool bEncrypt, uint64_t u64Serial, uint8_t u8Cmd, uint8_t u8Id, const uint8_t *pPayload, uint16_t u16PayloadLen, uint8_t *pFrame, uint16_t *u16FrameLen)
{
    Protocol_Header_t *pHeader;
    uint8_t *pCiphertext;
    uint16_t u16PlaintextLen;
    uint16_t u16PacketLen;
    uint16_t u16CrcEncrpyt;
    uint16_t u16CrcFrame;

    if ((pFrame == NULL) || (u16FrameLen == NULL))
    {
        return false;
    }

    pHeader = (Protocol_Header_t *)pFrame;
    pCiphertext = pFrame + sizeof(Protocol_Header_t);

    pHeader->u8StartOfFrame = PROTOCOL_START_OF_FRAME;
    pHeader->u8ModuleType = PROTOCOL_MODULE_TYPE;
    pHeader->u64ModuleSerial = u64Serial;
    pHeader->u8CmdCode = u8Cmd;
    pHeader->u8IdCode = u8Id;

    if ((pPayload == NULL) || (u16PayloadLen == 0))
    {
        pHeader->u16CiphertextLen = 0; // Payload = NULL
    }
    else
    {
        memmove(pCiphertext, &u16PayloadLen, sizeof(u16PayloadLen));           // Copy payload length to buffer
        memmove(pCiphertext + sizeof(u16PayloadLen), pPayload, u16PayloadLen); // Copy payload to buffer

        u16PlaintextLen = sizeof(u16PayloadLen) + u16PayloadLen;

        if (bEncrypt)
        {
            u16CrcEncrpyt = sys_crc16(pCiphertext, u16PlaintextLen);
            memmove(pCiphertext + u16PlaintextLen, &u16CrcEncrpyt, sizeof(u16CrcEncrpyt)); // Copy crc encrypt to buffer
            u16PlaintextLen += sizeof(u16CrcEncrpyt);
            pHeader->u16CiphertextLen = ALIGN16(u16PlaintextLen); // Length after encrypt
            aes_128_encrypt(au8Key, pCiphertext, u16PlaintextLen);
        }
        else
        {
            pHeader->u16CiphertextLen = u16PlaintextLen;
        }
    }

    u16PacketLen = sizeof(Protocol_Header_t) + pHeader->u16CiphertextLen;
    u16CrcFrame = sys_crc16(pFrame, u16PacketLen);
    memmove(pFrame + u16PacketLen, &u16CrcFrame, sizeof(u16CrcFrame));
    *u16FrameLen = u16PacketLen + sizeof(u16CrcFrame);

    return true;
}

Protocol_Err_Code_t drv_protocol_unpack(const uint8_t *pFrame, uint16_t u16FrameLen, uint64_t *u64Serial, uint8_t *u8Cmd, uint8_t *u8Id, uint8_t *pPayload, uint16_t *u16PayloadLen)
{
    const Protocol_Header_t *pHeader;
    const uint8_t *pCiphertext;
    uint16_t u16PacketLen;
    uint16_t u16CrcFrame;
    uint16_t u16CrcFrameCalc;
    uint16_t u16CrcEncrpyt;
    uint16_t u16CrcEncrpytCalc;

    if ((pFrame == NULL) || (u64Serial == NULL) || (u8Cmd == NULL) || (u8Id == NULL) || (pPayload == NULL) || (u16PayloadLen == NULL))
    {
        return PROTOCOL_ERR_NULL_POINTER;
    }

    if (u16FrameLen < sizeof(Protocol_Header_t) + 2)
    {
        return PROTOCOL_ERR_FRAME_INVALID;
    }

    pHeader = (const Protocol_Header_t *)pFrame;

    if (pHeader->u8StartOfFrame != PROTOCOL_START_OF_FRAME)
    {
        return PROTOCOL_ERR_FRAME_INVALID;
    }

    if (pHeader->u16CiphertextLen > u16FrameLen - sizeof(Protocol_Header_t) - sizeof(uint16_t))
    {
        return PROTOCOL_ERR_FRAME_INVALID;
    }

    u16PacketLen = u16FrameLen - sizeof(u16CrcFrame); // -2 bytes CRC
    u16CrcFrameCalc = sys_crc16(pFrame, u16PacketLen);
    memcpy(&u16CrcFrame, pFrame + u16PacketLen, sizeof(u16CrcFrame));
    if (u16CrcFrameCalc != u16CrcFrame)
    {
        return PROTOCOL_ERR_CRC16_INVALID;
    }

    *u64Serial = pHeader->u64ModuleSerial;
    *u8Cmd = pHeader->u8CmdCode;
    *u8Id = pHeader->u8IdCode;

    if (pHeader->u16CiphertextLen == 0)
    {
        *u16PayloadLen = 0;
        return PROTOCOL_ERR_SUCCESS;
    }

    pCiphertext = pFrame + sizeof(Protocol_Header_t);
    memcpy(pPayload, pCiphertext, pHeader->u16CiphertextLen); // Copy ciphertext to payload pointer

    // AES ciphertext length must be at least one block (16 bytes)
    if (pHeader->u16CiphertextLen >= 16)
    {
        aes_128_decrypt(au8Key, pPayload, pHeader->u16CiphertextLen);

        memcpy(u16PayloadLen, pPayload, sizeof(*u16PayloadLen));

        if ((uint32_t)*u16PayloadLen + sizeof(*u16PayloadLen) + sizeof(u16CrcEncrpyt) > pHeader->u16CiphertextLen)
        {
            return PROTOCOL_ERR_FRAME_INVALID;
        }

        memcpy(&u16CrcEncrpyt, &pPayload[sizeof(*u16PayloadLen) + *u16PayloadLen], sizeof(u16CrcEncrpyt));

        u16CrcEncrpytCalc = sys_crc16(pPayload, sizeof(*u16PayloadLen) + *u16PayloadLen);

        if (u16CrcEncrpytCalc != u16CrcEncrpyt)
        {
            return PROTOCOL_ERR_FRAME_INVALID;
        }
    }
    else
    {
        memcpy(u16PayloadLen, pPayload, sizeof(*u16PayloadLen));

        if ((uint32_t)*u16PayloadLen + sizeof(*u16PayloadLen) > pHeader->u16CiphertextLen)
        {
            return PROTOCOL_ERR_FRAME_INVALID;
        }
    }

    memmove(pPayload, pPayload + sizeof(*u16PayloadLen), *u16PayloadLen);
    return PROTOCOL_ERR_SUCCESS;
}