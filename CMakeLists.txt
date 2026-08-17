cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME WM03)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# ============================================================
# This code is copied from STM32CubeMX-generated sources.
# Re-copy it if the code is regenerated.
# ============================================================

# STM32CubeMX generated symbols (macros)
set(MX_Defines_Syms
    USE_HAL_DRIVER
    STM32L431xx
    $<$<CONFIG:Debug>:DEBUG>
)

# STM32CubeMX generated include paths
set(MX_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Inc
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Inc/Legacy
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32L4xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Include
)

# STM32CubeMX generated application sources
set(MX_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/iwdg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/stm32l4xx_it.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/stm32l4xx_hal_msp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/sysmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/syscalls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/startup_stm32l431xx.s
)

# STM32 HAL/LL Drivers
set(STM32_Drivers_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/Core/Src/system_stm32l4xx.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_iwdg.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_rcc_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_flash_ramfunc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_gpio.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_i2c_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_dma_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_pwr_ex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_cortex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/STM32L4xx_HAL_Driver/Src/stm32l4xx_hal_exti.c
)

# ============================================================
# This code is copied from STM32CubeMX-generated sources.
# No changes are needed here.
# ============================================================

add_library(stm32cubemx INTERFACE)
target_include_directories(stm32cubemx INTERFACE ${MX_Include_Dirs})
target_compile_definitions(stm32cubemx INTERFACE ${MX_Defines_Syms})

add_library(STM32_Drivers OBJECT)
target_sources(STM32_Drivers PRIVATE ${STM32_Drivers_Src})
target_link_libraries(STM32_Drivers PUBLIC stm32cubemx)

if((CMAKE_C_STANDARD EQUAL 90) OR (CMAKE_C_STANDARD EQUAL 99))
    message(ERROR "Generated code requires C11 or higher")
endif()

# ============================================================
# Build Boot and App
# ============================================================

# User source files
file(GLOB_RECURSE USER_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Users/*.c
)

# Boot sources
set(BOOT_SOURCES ${USER_SOURCES})
list(FILTER BOOT_SOURCES EXCLUDE REGEX ".*/app_main\\.c$")

# App sources
set(APP_SOURCES ${USER_SOURCES})
list(FILTER APP_SOURCES EXCLUDE REGEX ".*/boot_main\\.c$")

# User paths
set(APP_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/Users
    ${CMAKE_CURRENT_SOURCE_DIR}/Users/Main
    ${CMAKE_CURRENT_SOURCE_DIR}/Users/Configuration
)

# Create Boot and App executables
add_executable(WM03_boot ${BOOT_SOURCES} ${MX_Application_Src})
add_executable(WM03_app_a ${APP_SOURCES} ${MX_Application_Src})
add_executable(WM03_app_b ${APP_SOURCES} ${MX_Application_Src})

# Define build-specific macros
target_compile_definitions(WM03_boot PRIVATE IS_BOOT)
target_compile_definitions(WM03_app_a PRIVATE IS_APP)
target_compile_definitions(WM03_app_b PRIVATE IS_APP)

# Add user include directories
target_include_directories(WM03_boot PRIVATE ${APP_INCLUDE_DIRS})
target_include_directories(WM03_app_a PRIVATE ${APP_INCLUDE_DIRS})
target_include_directories(WM03_app_b PRIVATE ${APP_INCLUDE_DIRS})

# Link STM32CubeMX and STM32 driver libraries
target_link_libraries(WM03_boot PRIVATE stm32cubemx STM32_Drivers)
target_link_libraries(WM03_app_a PRIVATE stm32cubemx STM32_Drivers)
target_link_libraries(WM03_app_b PRIVATE stm32cubemx STM32_Drivers)

# Specify linker scripts and map files
target_link_options(WM03_boot PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/Linker/STM32L431XX_BOOT.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/Boot/WM03_boot.map
)
target_link_options(WM03_app_a PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/Linker/STM32L431XX_APPA.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/App_a/WM03_app_a.map
)
target_link_options(WM03_app_b PRIVATE
    -T${CMAKE_CURRENT_SOURCE_DIR}/Linker/STM32L431XX_APPB.ld
    -Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/App_b/WM03_app_b.map
)

# Set output directories
set_target_properties(WM03_boot PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Boot")
set_target_properties(WM03_app_a PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/App_a")
set_target_properties(WM03_app_b PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/App_b")

# Remove map files when cleaning the build
set_target_properties(WM03_boot PROPERTIES ADDITIONAL_CLEAN_FILES "WM03_boot.map;WM03_boot.hex;WM03_boot.bin")
set_target_properties(WM03_app_a PROPERTIES ADDITIONAL_CLEAN_FILES "WM03_app_a.map;WM03_app_a.hex;WM03_app_a.bin")
set_target_properties(WM03_app_b PROPERTIES ADDITIONAL_CLEAN_FILES "WM03_app_b.map;WM03_app_b.hex;WM03_app_b.bin")

# Generate HEX and BIN files for Boot
add_custom_command(TARGET WM03_boot POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:WM03_boot>
            $<TARGET_FILE_DIR:WM03_boot>/WM03_boot.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:WM03_boot>
            $<TARGET_FILE_DIR:WM03_boot>/WM03_boot.bin
    COMMENT "Generating WM03_boot.hex and WM03_boot.bin"
)
add_custom_command(TARGET WM03_app_a POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:WM03_app_a>
            $<TARGET_FILE_DIR:WM03_app_a>/WM03_app_a.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:WM03_app_a>
            $<TARGET_FILE_DIR:WM03_app_a>/WM03_app_a.bin
    COMMENT "Generating WM03_app_a.hex and WM03_app_a.bin"
)
add_custom_command(TARGET WM03_app_b POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex
            $<TARGET_FILE:WM03_app_b>
            $<TARGET_FILE_DIR:WM03_app_b>/WM03_app_b.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary
            $<TARGET_FILE:WM03_app_b>
            $<TARGET_FILE_DIR:WM03_app_b>/WM03_app_b.bin
    COMMENT "Generating WM03_app_b.hex and WM03_app_b.bin"
)