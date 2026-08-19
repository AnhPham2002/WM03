BUILD_DIR := build
CMAKE     := cmake
GENERATOR := Ninja
TOOLCHAIN := cmake/gcc-arm-none-eabi.cmake

BUILD_DEBUG := $(BUILD_DIR)/Debug
BUILD_RELEASE := $(BUILD_DIR)/Release

BOOT_ELF_DEBUG  := $(BUILD_DIR)/Debug/Boot/WM03_BOOT.elf
APPA_ELF_DEBUG  := $(BUILD_DIR)/Debug/App_A/WM03_APP_A.elf
APPB_ELF_DEBUG  := $(BUILD_DIR)/Debug/App_B/WM03_APP_B.elf

BOOT_ELF_RELEASE := $(BUILD_DIR)/Release/Boot/WM03_BOOT.elf
APPA_ELF_RELEASE := $(BUILD_DIR)/Release/App_A/WM03_APP_A.elf
APPB_ELF_RELEASE := $(BUILD_DIR)/Release/App_B/WM03_APP_B.elf

.PHONY: all
all: clean build

# ============================================================
# Build
# ============================================================

.PHONY: build build-d build-r \
		build-boot-d build-boot-r \
		build-app-a-d build-app-a-r \
		build-app-b-d build-app-b-r

build: build-d build-r

build-d: build-boot-d build-app-a-d build-app-b-d

build-r: build-boot-r build-app-a-r build-app-b-r

build-boot-d:
	$(CMAKE) -S . -B $(BUILD_DEBUG)/Boot -G $(GENERATOR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DWM03_BOOT=ON
	$(CMAKE) --build $(BUILD_DEBUG)/Boot

build-app-a-d:
	$(CMAKE) -S . -B $(BUILD_DEBUG)/App_A -G $(GENERATOR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DWM03_APP_A=ON
	$(CMAKE) --build $(BUILD_DEBUG)/App_A

build-app-b-d:
	$(CMAKE) -S . -B $(BUILD_DEBUG)/App_B -G $(GENERATOR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DWM03_APP_B=ON
	$(CMAKE) --build $(BUILD_DEBUG)/App_B

build-boot-r:
	$(CMAKE) -S . -B $(BUILD_RELEASE)/Boot -G $(GENERATOR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DWM03_BOOT=ON
	$(CMAKE) --build $(BUILD_RELEASE)/Boot

build-app-a-r:
	$(CMAKE) -S . -B $(BUILD_RELEASE)/App_A -G $(GENERATOR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DWM03_APP_A=ON
	$(CMAKE) --build $(BUILD_RELEASE)/App_A

build-app-b-r:
	$(CMAKE) -S . -B $(BUILD_RELEASE)/App_B -G $(GENERATOR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DWM03_APP_B=ON
	$(CMAKE) --build $(BUILD_RELEASE)/App_B

# ============================================================
# Flash: (default: boot + app_a (release))
# ============================================================

.PHONY: flash
flash: build-r
# 	STM32_Programmer_CLI -c port=SWD mode=UR -e all
	STM32_Programmer_CLI -c port=SWD mode=UR -w "$(BOOT_ELF_RELEASE)" -v
	STM32_Programmer_CLI -c port=SWD mode=UR -w "$(APPA_ELF_RELEASE)" -v
	STM32_Programmer_CLI -c port=SWD -rst

flash-d: build-d
# 	STM32_Programmer_CLI -c port=SWD mode=UR -e all
	STM32_Programmer_CLI -c port=SWD mode=UR -w "$(BOOT_ELF_DEBUG)" -v
	STM32_Programmer_CLI -c port=SWD mode=UR -w "$(APPA_ELF_DEBUG)" -v
	STM32_Programmer_CLI -c port=SWD -rst

# ============================================================
# Clean
# ============================================================

.PHONY: clean
clean:
	$(CMAKE) --build $(BUILD_DIR)/Debug/Boot --target clean || true
	$(CMAKE) --build $(BUILD_DIR)/Debug/App_A --target clean || true
	$(CMAKE) --build $(BUILD_DIR)/Debug/App_B --target clean || true
	$(CMAKE) --build $(BUILD_DIR)/Release/Boot --target clean || true
	$(CMAKE) --build $(BUILD_DIR)/Release/App_A --target clean || true
	$(CMAKE) --build $(BUILD_DIR)/Release/App_B --target clean || true

# ============================================================
# Remove all build files
# ============================================================

.PHONY: clear
clear:
	$(CMAKE) -E remove_directory $(BUILD_DIR)