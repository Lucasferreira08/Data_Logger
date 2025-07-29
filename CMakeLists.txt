
cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(Data_Logger C CXX ASM)

pico_sdk_init()

add_subdirectory(libs/FatFs_SPI)
add_subdirectory(libs/Mpu6050)
add_subdirectory(libs/Bitdoglab_pico)
add_subdirectory(libs/Ssd1306)

add_executable(${PROJECT_NAME} Data_Logger.c )

pico_set_program_name(${PROJECT_NAME} "Data_Logger")
pico_set_program_version(${PROJECT_NAME} "0.1")

pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)

target_link_libraries(${PROJECT_NAME}
        pico_stdlib)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/libs/Bitdoglab_pico
        ${CMAKE_CURRENT_LIST_DIR}/libs/FatFs_SPI
        ${CMAKE_CURRENT_LIST_DIR}/libs/Mpu6050
        ${CMAKE_CURRENT_LIST_DIR}/libs/Ssd1306
)

target_link_libraries(${PROJECT_NAME} 
        FatFs_SPI
        Mpu6050
        Bitdoglab_pico
        Ssd1306
        )

pico_add_extra_outputs(${PROJECT_NAME})

