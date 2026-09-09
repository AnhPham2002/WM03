#pragma once

#include "Ti_aes_128.h"

#include <stdint.h>
#include <string.h>

/**
 * @brief Encrypt data using AES-128.
 *
 * Pads the data to a multiple of 16 bytes using 0x00 before encryption.
 *
 * @param[in]     au8Key      AES-128 encryption key.
 * @param[in,out] pData       Data buffer.
 * @param[in]     u16DataSize Data size in bytes.
 */
void aes_128_encrypt(const uint8_t au8Key[16], uint8_t *pData, uint16_t u16DataSize);

/**
 * @brief Decrypt data using AES-128.
 *
 * @param[in]     au8Key      AES-128 decryption key.
 * @param[in,out] pData       Data buffer.
 * @param[in]     u16DataSize Encrypted data size in bytes.
 */
void aes_128_decrypt(const uint8_t au8Key[16], uint8_t *pData, uint16_t u16DataSize);
