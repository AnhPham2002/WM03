#pragma once

#include "app_cellular.h"
#include "app_latch.h"

#define PUSH_RETRY 3
#define PUSH_RESPONSE_TIMOUT 5000

typedef enum
{
    PUSH_STEP_IDLE = 0,
    PUSH_STEP_SEND_INFO,
    PUSH_STEP_SEND_LATCH,
    PUSH_STEP_SEND_EVENT,
    PUSH_STEP_UPDATE_METADATA
} Push_Step_t;

typedef enum
{
    PUSH_ERROR_NONE = 0,
    PUSH_ERROR_DISCONNECT,
    PUSH_ERROR_PACK,
    PUSH_ERROR_INFO,
    PUSH_ERROR_LATCH,
    PUSH_ERROR_EVENT,
    PUSH_ERROR_REQUEST_PROCESS
} Push_Error_t;

/**
 * @brief Execute the data push process.
 *
 * Manages the cellular connection and sequentially pushes device information,
 * latch data and event data to the server. Updates the storage load indices
 * after successful transmission or when the retry limit is reached.
 */
void app_push_execute(void);

/**
 * @brief Activate the periodic data push process.
 *
 * Sets the data push request flag.
 */
void app_push_activate(void);