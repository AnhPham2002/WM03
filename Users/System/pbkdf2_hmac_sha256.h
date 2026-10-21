#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief PBKDF2 operation status.
 */
typedef enum
{
    PBKDF2_STATUS_OK = 0,
    PBKDF2_STATUS_INVALID_PARAM,
    PBKDF2_STATUS_CRYPTO_ERROR
} PBKDF2_Status_t;

/**
 * @brief Maximum supported salt length in bytes.
 */
#define PBKDF2_MAX_SALT_SIZE 64U

/**
 * @brief SHA-256 digest length in bytes.
 */
#define PBKDF2_SHA256_SIZE 32U

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
 *
 * @note The salt length must not exceed PBKDF2_MAX_SALT_SIZE.
 * @note The caller is responsible for generating a unique, random salt.
 */
PBKDF2_Status_t pbkdf2_hmac_sha256(const uint8_t *pu8Password, size_t uPasswordLen, const uint8_t *pu8Salt, size_t uSaltLen, uint32_t u32Iterations, uint8_t *pu8DerivedKey, size_t uDerivedKeyLen);