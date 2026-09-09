#include "aes128.h"

#include <math.h>
#include <string.h>

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief Encrypt data using AES-128.
 *
 * @param[in]  au8Key AES-128 encryption key.
 * @param[in,out] data Data to encrypt.
 */
static void aes_128_enc(const uint8_t au8Key[16], uint8_t *data);

/**
 * @brief Decrypt data using AES-128.
 *
 * @param[in]  au8Key AES-128 decryption key.
 * @param[in,out] data Data to decrypt.
 */
static void aes_128_dec(const uint8_t au8Key[16], uint8_t *data);

/**
 * @brief Encrypt multiple 16-byte blocks using AES-128.
 *
 * @param[in]     au8Key AES-128 encryption key.
 * @param[in,out] data   Data buffer.
 * @param[in]     num    Number of 16-byte blocks.
 */
static void aes_128_enc_num(const uint8_t au8Key[16], uint8_t *data, uint8_t num);


/**
 * @brief Decrypt multiple 16-byte blocks using AES-128.
 *
 * @param[in]     au8Key AES-128 decryption key.
 * @param[in,out] data   Data buffer.
 * @param[in]     num    Number of 16-byte blocks.
 */
static void aes_128_dec_num(const uint8_t au8Key[16], uint8_t *data, uint8_t num);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void aes_128_encrypt(const uint8_t au8Key[16], uint8_t *pData, uint16_t u16DataSize)
{
	uint8_t u8BlockNum;
	uint8_t u8PaddingLen;

	// Calculate number of blocks (ceil)
	u8BlockNum = (uint8_t)((u16DataSize + 15U) / 16U);

	// Padding length
	u8PaddingLen = (uint8_t)((u8BlockNum * 16U) - u16DataSize);

	// Padding with 0x00
	for (uint8_t i = 0U; i < u8PaddingLen; i++)
    {
        pData[u16DataSize + i] = 0x00U;
    }

	// Encrypt all blocks
	aes_128_enc_num(au8Key, pData, u8BlockNum);
}

void aes_128_decrypt(const uint8_t au8Key[16], uint8_t *pData, uint16_t u16DataSize)
{
	uint8_t u8BlockNum;

    // Calculate number of blocks (ceil)
    u8BlockNum = (uint8_t)((u16DataSize + 15U) / 16U);

    // Decrypt all blocks
    aes_128_dec_num(au8Key, pData, u8BlockNum);
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static void aes_128_enc(const uint8_t au8Key[16], uint8_t *pData)
{
	uint8_t key[16];

	memcpy(key, au8Key, sizeof(key));
	aes_enc_dec(pData, key, 0);
}

static void aes_128_dec(const uint8_t au8Key[16], uint8_t *pData)
{
	uint8_t key[16];

	memcpy(key, au8Key, sizeof(key));
	aes_enc_dec(pData, key, 1);
}

static void aes_128_enc_num(const uint8_t au8Key[16], uint8_t *pData, uint8_t u8Num)
{
	for(uint8_t i = 0; i < u8Num; i++)
	{
		aes_128_enc(au8Key, &pData[i * 16U]);
	}
}

static void aes_128_dec_num(const uint8_t au8Key[16], uint8_t *pData, uint8_t u8Num)
{
	for(uint8_t i = 0; i < u8Num; i++)
	{
		aes_128_dec(au8Key, &pData[i * 16U]);
	}
}