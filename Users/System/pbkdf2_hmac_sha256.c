#include "pbkdf2_hmac_sha256.h"

#include <string.h>

#include "mac/cmox_hmac.h"

/**
 * @brief Derive a key using PBKDF2-HMAC-SHA-256.
 *
 * @param[in]  pu8Password      Password buffer.
 * @param[in]  uPasswordLen     Password length in bytes.
 * @param[in]  pu8Salt          Salt buffer.
 * @param[in]  uSaltLen         Salt length in bytes.
 * @param[in]  u32Iterations    Number of PBKDF2 iterations; must be >= 1.
 * @param[out] pu8DerivedKey    Output buffer for the derived key.
 * @param[in]  uDerivedKeyLen   Requested derived-key length in bytes.
 *
 * @return PBKDF2_STATUS_OK on success; otherwise an error status.
 */
PBKDF2_Status_t pbkdf2_hmac_sha256(const uint8_t *pu8Password, size_t uPasswordLen, const uint8_t *pu8Salt, size_t uSaltLen, uint32_t u32Iterations, uint8_t *pu8DerivedKey, size_t uDerivedKeyLen)
{
    uint8_t au8Input[PBKDF2_MAX_SALT_SIZE + 4U];
    uint8_t au8U[PBKDF2_SHA256_SIZE];
    uint8_t au8T[PBKDF2_SHA256_SIZE];
    uint8_t au8NextU[PBKDF2_SHA256_SIZE];
    size_t uComputedTagLen;
    size_t uBytesToCopy;
    size_t uOffset = 0U;
    uint32_t u32BlockIndex = 1U;
    uint32_t u32Iteration;
    size_t i;
    cmox_mac_retval_t eCryptoStatus;

    if ((pu8Password == NULL) || (uPasswordLen == 0U) || (pu8Salt == NULL) || (uSaltLen == 0U) || (uSaltLen > PBKDF2_MAX_SALT_SIZE) || (u32Iterations == 0U) || (pu8DerivedKey == NULL) ||
        (uDerivedKeyLen == 0U))
    {
        return PBKDF2_STATUS_INVALID_PARAM;
    }

    memcpy(au8Input, pu8Salt, uSaltLen);

    while (uOffset < uDerivedKeyLen)
    {
        /* Append INT(block_index), encoded as a 32-bit big-endian integer. */
        au8Input[uSaltLen] = (uint8_t)(u32BlockIndex >> 24U);
        au8Input[uSaltLen + 1U] = (uint8_t)(u32BlockIndex >> 16U);
        au8Input[uSaltLen + 2U] = (uint8_t)(u32BlockIndex >> 8U);
        au8Input[uSaltLen + 3U] = (uint8_t)u32BlockIndex;

        /* U1 = PRF(password, salt || INT(block_index)). */
        uComputedTagLen = 0U;
        eCryptoStatus = cmox_mac_compute(CMOX_HMAC_SHA256_ALGO, au8Input, uSaltLen + 4U, pu8Password, uPasswordLen, NULL, 0U, au8U, PBKDF2_SHA256_SIZE, &uComputedTagLen);

        if ((eCryptoStatus != CMOX_MAC_SUCCESS) || (uComputedTagLen != PBKDF2_SHA256_SIZE))
        {
            return PBKDF2_STATUS_CRYPTO_ERROR;
        }

        memcpy(au8T, au8U, PBKDF2_SHA256_SIZE);

        /* T = U1 XOR U2 XOR ... XOR Uc. */
        for (u32Iteration = 1U; u32Iteration < u32Iterations; u32Iteration++)
        {
            uComputedTagLen = 0U;
            eCryptoStatus = cmox_mac_compute(CMOX_HMAC_SHA256_ALGO, au8U, PBKDF2_SHA256_SIZE, pu8Password, uPasswordLen, NULL, 0U, au8NextU, PBKDF2_SHA256_SIZE, &uComputedTagLen);

            if ((eCryptoStatus != CMOX_MAC_SUCCESS) || (uComputedTagLen != PBKDF2_SHA256_SIZE))
            {
                return PBKDF2_STATUS_CRYPTO_ERROR;
            }

            memcpy(au8U, au8NextU, PBKDF2_SHA256_SIZE);

            for (i = 0U; i < PBKDF2_SHA256_SIZE; i++)
            {
                au8T[i] ^= au8U[i];
            }
        }

        uBytesToCopy = uDerivedKeyLen - uOffset;

        if (uBytesToCopy > PBKDF2_SHA256_SIZE)
        {
            uBytesToCopy = PBKDF2_SHA256_SIZE;
        }

        memcpy(&pu8DerivedKey[uOffset], au8T, uBytesToCopy);
        uOffset += uBytesToCopy;

        if (uOffset < uDerivedKeyLen)
        {
            if (u32BlockIndex == UINT32_MAX)
            {
                return PBKDF2_STATUS_INVALID_PARAM;
            }

            u32BlockIndex++;
        }
    }

    /* Clear intermediate key material. */
    memset(au8Input, 0, sizeof(au8Input));
    memset(au8U, 0, sizeof(au8U));
    memset(au8T, 0, sizeof(au8T));
    memset(au8NextU, 0, sizeof(au8NextU));

    return PBKDF2_STATUS_OK;
}