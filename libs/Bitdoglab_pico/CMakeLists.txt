add_library(Bitdoglab_pico INTERFACE)
target_sources(Bitdoglab_pico INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/bitdoglab_pico.c
)
target_include_directories(Bitdoglab_pico INTERFACE
)
target_link_libraries(Bitdoglab_pico INTERFACE
        pico_stdlib
        hardware_i2c
        hardware_pwm
        hardware_clocks
)
