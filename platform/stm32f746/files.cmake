
# STM32F746 platform sources

set(BL_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platform/stm32f746")

# CubeMX generated HAL, USB, Ethernet, LwIP and startup sources
add_subdirectory(
    "${BL_PLATFORM_DIR}/cmake/stm32cubemx"
    "${CMAKE_CURRENT_BINARY_DIR}/stm32f746_cubemx"
)

# Apply Cortex-M7 and FPU flags to CubeMX libraries too
target_compile_options(stm32cubemx INTERFACE ${CPU_FLAGS})

# Common bootloader sources
target_sources(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/user/app/bootloader_app.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/user/drv/bl_commands.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/user/drv/bl_comm.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/user/drv/bl_drv.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/user/drv/bl_usb.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/user/drv/bl_gpio.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/user/drv/bl_uart.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/user/drv/ring_buffer.c"
)

target_link_libraries(${PROJECT_NAME} m)
