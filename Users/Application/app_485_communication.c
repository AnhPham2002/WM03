#include "app_485_communication.h"

static uint8_t au8TxBuffer[512];
static uint16_t u16TxBufferLen;
static uint8_t au8RxBuffer[1600];
static uint16_t u16RxBufferLen;

void app_485_communication_execute(void)
{
    sv_485_communication_power_process();
    
    if (sv_485_communication_receive(au8RxBuffer, &u16RxBufferLen))
    {
        app_protocol_process(PROTOCOL_DATA_SOURCE_RS485, au8RxBuffer, u16RxBufferLen, au8TxBuffer, &u16TxBufferLen);

        sv_485_communication_init();
        sv_485_communication_power_process();

        sv_485_communication_send(au8TxBuffer, u16TxBufferLen);
    }
}