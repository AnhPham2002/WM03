#pragma once

#define RAM_NOINIT __attribute__((section(".ram_noinit")))

#define RAM_NOINIT_OTA_METADATA_MAGIC_NUMBER 0x11223344
#define RAM_NOINIT_OTA_METADATA_ADDRESS 0x10000000