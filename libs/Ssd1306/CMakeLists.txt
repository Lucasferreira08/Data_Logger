add_library(Ssd1306 INTERFACE)
target_sources(Ssd1306 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/ssd1306.c
)
target_include_directories(Ssd1306 INTERFACE
    # ff15
    # sd_driver
    # include
)
target_link_libraries(Ssd1306 INTERFACE
        pico_stdlib
        hardware_i2c
)
