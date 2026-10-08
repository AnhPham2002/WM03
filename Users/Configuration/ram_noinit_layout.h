#pragma once

#include <stdint.h>
#include <stdbool.h>

#define RAM_NOINIT __attribute__((section(".ram_noinit")))

#define RAM_NOINIT_OTA_METADATA_MAGIC_NUMBER 0x11223344
#define RAM_NOINIT_OTA_METADATA_ADDRESS 0x10000000

typedef struct
{
    uint32_t u32Magic;
    uint8_t au8Version[12];
    bool bFirmwareUpdated;
    uint16_t u16PacketIndex;
    uint32_t u32Size;
    uint32_t u32Crc;
} Ota_Metadata_t;