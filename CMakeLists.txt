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
if (WM03_BOOT)
    set(CMAKE_PROJECT_NAME WM03_BOOT)
elseif (WM03_APP_A)
    set(CMAKE_PROJECT_NAME WM03_APP_A)
elseif (WM03_APP_B)
    set(CMAKE_PROJECT_NAME WM03_APP_B)
endif()

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# User source files
file(GLOB_RECURSE USER_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/Users/*.c
)

# Select firmware sources
if (WM03_BOOT)
    set(FIRMWARE_SOURCES ${USER_SOURCES})
    list(FILTER FIRMWARE_SOURCES EXCLUDE REGEX ".*/app_main\\.c$")
elseif(WM03_APP_A OR WM03_APP_B)
    set(FIRMWARE_SOURCES ${USER_SOURCES})
    list(FILTER FIRMWARE_SOURCES EXCLUDE REGEX ".*/boot_main\\.c$")
endif()

# Firmware compile definitions
if (WM03_BOOT)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE IS_BOOT)
elseif(WM03_APP_A OR WM03_APP_B)
    target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE IS_APP)
endif()

# User paths
set(FIRMWARE_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/Users
    ${CMAKE_CURRENT_SOURCE_DIR}/Users/Main
    ${CMAKE_CURRENT_SOURCE_DIR}/Users/System
    ${CMAKE_CURRENT_SOURCE_DIR}/Users/Configuration
    ${CMAKE_CURRENT_SOURCE_DIR}/Users/Driver/Internal
    ${CMAKE_CURRENT_SOURCE_DIR}/Users/Driver/External
)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    ${FIRMWARE_SOURCES}
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    ${FIRMWARE_INCLUDE_DIRS}
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    # Add user defined libraries
)

# Select linker script
if (WM03_BOOT)
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Linker/STM32L431XX_BOOT.ld)
elseif(WM03_APP_A)
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Linker/STM32L431XX_APPA.ld)
elseif(WM03_APP_B)
    set(LINKER_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/Linker/STM32L431XX_APPB.ld)
endif()

# Specify linker scripts and map files
target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
    -T${LINKER_SCRIPT}
    -Wl,-Map=${CMAKE_PROJECT_NAME}.map
    -Wl,--gc-sections
)

# Generate BIN and HEX files from ELF
add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD

    COMMAND ${CMAKE_OBJCOPY}
        -O binary
        $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.bin

    COMMAND ${CMAKE_OBJCOPY}
        -O ihex
        $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
        ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.hex

    COMMENT "Generating BIN and HEX files"
)

# Remove wrong libob.a dependency when using C++
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)