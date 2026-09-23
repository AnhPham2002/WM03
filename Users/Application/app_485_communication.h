#pragma once

#include "sv_485_communication.h"
#include "app_protocol.h"

/**
 * @brief Execute RS485 communication process.
 *
 * Receives and processes the incoming protocol frame, manages RS485 power,
 * and sends the response frame.
 */
void app_485_communication_execute(void);