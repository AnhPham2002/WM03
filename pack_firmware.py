import re
import struct
from pathlib import Path


BASE_DIR = Path(__file__).resolve().parent

FLASH_LAYOUT_HEADER = BASE_DIR / "Users" / "Configuration" / "flash_layout.h"
APP_STORAGE_HEADER = BASE_DIR / "Users" / "Application" / "app_storage.h"

BOOT_BIN = BASE_DIR / "build" / "Release" / "Boot" / "WM03_BOOT.bin"
APP_A_BIN = BASE_DIR / "build" / "Release" / "App_A" / "WM03_APP_A.bin"
APP_B_BIN = BASE_DIR / "build" / "Release" / "App_B" / "WM03_APP_B.bin"

OUTPUT_BIN_FLASH = BASE_DIR / "build" / "Release" / "WM03_Firmware_Flash.bin"
OUTPUT_BIN_OTA = BASE_DIR / "build" / "Release" / "WM03_Firmware_Ota.bin"


def get_define(name):
    text = FLASH_LAYOUT_HEADER.read_text()

    match = re.search(rf"#define\s+{name}\s+(0x[0-9A-Fa-f]+|\d+)", text)

    if match is None:
        raise ValueError(f"Cannot find {name}")

    return int(match.group(1), 0)


def get_define_string(file_path, name):
    text = file_path.read_text()

    match = re.search(rf'#define\s+{name}\s+"([^"]*)"', text)

    if match is None:
        raise ValueError(f"Cannot find {name} in {file_path}")

    return match.group(1)


def get_enum_value(file_path, enum_name, item_name):
    text = file_path.read_text()

    match = re.search(
        rf'typedef\s+enum\s*\{{(.*?)\}}\s*{enum_name}\s*;',
        text,
        re.DOTALL
    )

    if match is None:
        raise ValueError(f"Cannot find enum {enum_name} in {file_path}")

    enum_body = match.group(1)

    value = -1

    for item in enum_body.split(","):
        item = item.strip()

        if not item:
            continue

        if "=" in item:
            name, value_str = map(str.strip, item.split("=", 1))
            value = int(value_str, 0)
        else:
            name = item.strip()
            value += 1

        if name == item_name:
            return value

    raise ValueError(f"Cannot find {item_name} in enum {enum_name}")


def crc32(data: bytes, init: int = 0xFFFFFFFF) -> int:
    poly = 0x04C11DB7
    crc = init & 0xFFFFFFFF

    def crc_word_update(crc_val: int, word: int) -> int:
        crc_val ^= word & 0xFFFFFFFF

        for _ in range(32):
            if crc_val & 0x80000000:
                crc_val = ((crc_val << 1) ^ poly) & 0xFFFFFFFF
            else:
                crc_val = (crc_val << 1) & 0xFFFFFFFF

        return crc_val

    n = len(data)
    i = 0

    while i + 4 <= n:
        word = (
            (data[i] << 24)
            | (data[i + 1] << 16)
            | (data[i + 2] << 8)
            | data[i + 3]
        )

        crc = crc_word_update(crc, word)
        i += 4

    rem = n - i

    if rem:
        b0 = data[i] if rem >= 1 else 0
        b1 = data[i + 1] if rem >= 2 else 0
        b2 = data[i + 2] if rem >= 3 else 0

        word = (b0 << 24) | (b1 << 16) | (b2 << 8)
        crc = crc_word_update(crc, word)

    return crc & 0xFFFFFFFF


def merge_binary(image, binary_path, start_address, image_start_address):
    data = binary_path.read_bytes()

    offset = start_address - image_start_address

    if offset < 0:
        raise ValueError(f"Invalid start address: 0x{start_address:08X}")

    if offset + len(data) > len(image):
        raise ValueError(f"{binary_path} exceeds image range")

    image[offset:offset + len(data)] = data

    print(f"{binary_path}")
    print(f"  Start : 0x{start_address:08X}")
    print(f"  Size  : {len(data)} bytes")

FIRMWARE_METADATA_SEQUENCE_OFFSET = 0
FIRMWARE_METADATA_VERSION_OFFSET = 4
FIRMWARE_METADATA_STATUS_OFFSET = 16
FIRMWARE_METADATA_SIZE_OFFSET = 20
FIRMWARE_METADATA_CRC_OFFSET = 24

FIRMWARE_METADATA_SIZE = 28
FIRMWARE_VERSION_SIZE = 12

def update_firmware_metadata(image, binary_path, metadata_address, image_start_address):
    data = binary_path.read_bytes()

    metadata_offset = metadata_address - image_start_address

    if metadata_offset < 0:
        raise ValueError(f"Invalid metadata address: 0x{metadata_address:08X}")

    if metadata_offset + FIRMWARE_METADATA_SIZE > len(image):
        raise ValueError("Metadata exceeds image range")

    firmware_size = len(data)
    firmware_crc32 = crc32(data)

    version_data = FIRMWARE_VERSION.encode("ascii")

    if len(version_data) >= FIRMWARE_VERSION_SIZE:
        raise ValueError(f"FIRMWARE_VERSION is too long: {FIRMWARE_VERSION}")

    struct.pack_into("<I", image, metadata_offset + FIRMWARE_METADATA_SEQUENCE_OFFSET, 0)

    image[
        metadata_offset + FIRMWARE_METADATA_VERSION_OFFSET:
        metadata_offset + FIRMWARE_METADATA_VERSION_OFFSET + FIRMWARE_VERSION_SIZE
    ] = b"\x00" * FIRMWARE_VERSION_SIZE

    image[
        metadata_offset + FIRMWARE_METADATA_VERSION_OFFSET:
        metadata_offset + FIRMWARE_METADATA_VERSION_OFFSET + len(version_data)
    ] = version_data

    struct.pack_into("<I", image, metadata_offset + FIRMWARE_METADATA_STATUS_OFFSET, FIRMWARE_VALID)
    struct.pack_into("<I", image, metadata_offset + FIRMWARE_METADATA_SIZE_OFFSET, firmware_size)
    struct.pack_into("<I", image, metadata_offset + FIRMWARE_METADATA_CRC_OFFSET, firmware_crc32)

    print(f"{binary_path}")
    print(f"  Metadata : 0x{metadata_address:08X}")
    print(f"  Version  : {FIRMWARE_VERSION}")
    print(f"  Size     : {firmware_size} bytes")
    print(f"  CRC32    : 0x{firmware_crc32:08X}")


BOOTLOADER_START_ADDR = get_define("BOOTLOADER_START_ADDR")
SLOT_A_METADATA_ADDR = get_define("SLOT_A_METADATA_ADDR")
SLOT_A_START_ADDR = get_define("SLOT_A_START_ADDR")
SLOT_B_METADATA_ADDR = get_define("SLOT_B_METADATA_ADDR")
SLOT_B_START_ADDR = get_define("SLOT_B_START_ADDR")

METADATA_MAX_SIZE = 2 * 1024
FIRMWARE_MAX_SIZE = 110 * 1024

FLASH_START_ADDR = BOOTLOADER_START_ADDR
SLOT_A_END_ADDR = SLOT_A_START_ADDR + FIRMWARE_MAX_SIZE
SLOT_B_END_ADDR = SLOT_B_START_ADDR + FIRMWARE_MAX_SIZE

FIRMWARE_VERSION = get_define_string(APP_STORAGE_HEADER, "FIRMWARE_VERSION")
FIRMWARE_VALID = get_enum_value(FLASH_LAYOUT_HEADER, "Firmware_Status_t", "FIRMWARE_VALID")


def main():
    print("========== FIRMWARE PACK ==========")
    print(f"Firmware version : {FIRMWARE_VERSION}")
    print()

    # ============================================================
    # FILE 1: FULL FLASH IMAGE
    # ============================================================

    image_flash_start_address = BOOTLOADER_START_ADDR
    image_flash_size = 256 * 1024
    image_flash = bytearray([0xFF] * image_flash_size)

    print("----- WM03_Firmware_Flash.bin -----")
    print(f"Start : 0x{image_flash_start_address:08X}")
    print(f"Size  : {image_flash_size} bytes")
    print()

    merge_binary(
        image_flash,
        BOOT_BIN,
        BOOTLOADER_START_ADDR,
        image_flash_start_address
    )

    merge_binary(
        image_flash,
        APP_A_BIN,
        SLOT_A_START_ADDR,
        image_flash_start_address
    )

    print()

    update_firmware_metadata(
        image_flash,
        APP_A_BIN,
        SLOT_A_METADATA_ADDR,
        image_flash_start_address
    )

    OUTPUT_BIN_FLASH.write_bytes(image_flash)

    print()
    print(f"Output : {OUTPUT_BIN_FLASH}")
    print(f"Size   : {len(image_flash)} bytes")
    print()

    # ============================================================
    # FILE 2: OTA IMAGE
    # ============================================================

    image_ota_start_address = SLOT_A_METADATA_ADDR
    image_ota_end_address = SLOT_B_END_ADDR

    image_ota_size = image_ota_end_address - image_ota_start_address
    image_ota = bytearray([0xFF] * image_ota_size)

    print("----- WM03_Firmware_Ota.bin -----")
    print(f"Start : 0x{image_ota_start_address:08X}")
    print(f"End   : 0x{image_ota_end_address:08X}")
    print(f"Size  : {image_ota_size} bytes")
    print()

    merge_binary(
        image_ota,
        APP_A_BIN,
        SLOT_A_START_ADDR,
        image_ota_start_address
    )

    merge_binary(
        image_ota,
        APP_B_BIN,
        SLOT_B_START_ADDR,
        image_ota_start_address
    )

    print()

    update_firmware_metadata(
        image_ota,
        APP_A_BIN,
        SLOT_A_METADATA_ADDR,
        image_ota_start_address
    )

    update_firmware_metadata(
        image_ota,
        APP_B_BIN,
        SLOT_B_METADATA_ADDR,
        image_ota_start_address
    )

    OUTPUT_BIN_OTA.write_bytes(image_ota)

    print()
    print(f"Output : {OUTPUT_BIN_OTA}")
    print(f"Size   : {len(image_ota)} bytes")
    print()

    print("========== PACK COMPLETE ==========")


if __name__ == "__main__":
    main()