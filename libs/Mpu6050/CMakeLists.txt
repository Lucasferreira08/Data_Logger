add_library(Mpu6050 INTERFACE)
target_sources(Mpu6050 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/mpu6050.c
)
target_include_directories(Mpu6050 INTERFACE
    # ff15
    # sd_driver
    # include
)
target_link_libraries(Mpu6050 INTERFACE
        pico_stdlib
        hardware_i2c
)
