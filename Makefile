BUILD_DIR := build
CMAKE     := cmake
GENERATOR := Ninja
TOOLCHAIN := cmake/gcc-arm-none-eabi.cmake

BUILD_DEBUG := $(BUILD_DIR)/Debug
BUILD_RELEASE := $(BUILD_DIR)/Release

BOOT_ELF_DEBUG    := $(BUILD_DEBUG)/Boot/WM03_boot.elf
APPA_ELF_DEBUG    := $(BUILD_DEBUG)/App_a/WM03_app_a.elf
APPB_ELF_DEBUG    := $(BUILD_DEBUG)/App_b/WM03_app_b.elf
BOOT_ELF_RELEASE  := $(BUILD_RELEASE)/Boot/WM03_boot.elf
APPA_ELF_RELEASE  := $(BUILD_RELEASE)/App_a/WM03_app_a.elf
APPB_ELF_RELEASE  := $(BUILD_RELEASE)/App_b/WM03_app_b.elf

.PHONY: all
all: clean debug release

# ============================================================
# Build
# ============================================================

.PHONY: build build-d build-r
build: build-d build-r
build-d:
	$(CMAKE) -S . -B $(BUILD_DIR)/Debug -G Ninja \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN)
	$(CMAKE) --build $(BUILD_DIR)/Debug
build-r:
	$(CMAKE) -S . -B $(BUILD_DIR)/Release -G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN)
	$(CMAKE) --build $(BUILD_DIR)/Release

# ============================================================
# Flash: (default: boot + app_a (release))
# ============================================================

.PHONY: flash
flash: build-r
	STM32_Programmer_CLI -c port=SWD -w "$(BOOT_ELF_RELEASE)" -rst
	STM32_Programmer_CLI -c port=SWD -w "$(APPA_ELF_RELEASE)" -rst

flash-d: build-d
	STM32_Programmer_CLI -c port=SWD -w "$(BOOT_ELF_DEBUG)" -rst
	STM32_Programmer_CLI -c port=SWD -w "$(APPA_ELF_DEBUG)" -rst

# ============================================================
# Clean
# ============================================================

.PHONY: clean
clean:
	-$(CMAKE) --build $(BUILD_DIR)/Debug --target clean
	-$(CMAKE) --build $(BUILD_DIR)/Release --target clean

# ============================================================
# Remove all build files
# ============================================================

.PHONY: clear
clear:
	$(CMAKE) -E remove_directory $(BUILD_DIR)