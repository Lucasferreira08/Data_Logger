# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(Data_Logger C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_subdirectory(libs/FatFs_SPI)
add_subdirectory(libs/Mpu6050)
add_subdirectory(libs/Bitdoglab_pico)
add_subdirectory(libs/Ssd1306)

# Add executable. Default name is the project name, version 0.1

add_executable(Data_Logger Data_Logger.c )

pico_set_program_name(Data_Logger "Data_Logger")
pico_set_program_version(Data_Logger "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(Data_Logger 0)
pico_enable_stdio_usb(Data_Logger 1)

# Add the standard library to the build
target_link_libraries(Data_Logger
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(Data_Logger PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/libs/Bitdoglab_pico
        ${CMAKE_CURRENT_LIST_DIR}/libs/FatFs_SPI
        ${CMAKE_CURRENT_LIST_DIR}/libs/Mpu6050
        ${CMAKE_CURRENT_LIST_DIR}/libs/Ssd1306
)

# Add any user requested libraries
target_link_libraries(Data_Logger 
        FatFs_SPI
        Mpu6050
        Bitdoglab_pico
        Ssd1306
        )

pico_add_extra_outputs(Data_Logger)

