
# STM32F746 compile and linker configuration

set(BL_PLATFORM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/platform/stm32f746")
set(BL_LINKER_SCRIPT "${BL_PLATFORM_DIR}/STM32F746xx_FLASH.ld")

set_target_properties(${PROJECT_NAME} PROPERTIES
    SUFFIX ".elf"
    LINK_DEPENDS "${BL_LINKER_SCRIPT}"
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    "${BL_PLATFORM_DIR}/Core/Inc"
    "${BL_PLATFORM_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc"
    "${BL_PLATFORM_DIR}/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy"
    "${BL_PLATFORM_DIR}/Drivers/CMSIS/Device/ST/STM32F7xx/Include"
    "${BL_PLATFORM_DIR}/Drivers/CMSIS/Include"
    "${BL_PLATFORM_DIR}/USB_DEVICE/App"
    "${BL_PLATFORM_DIR}/USB_DEVICE/Target"
    "${BL_PLATFORM_DIR}/Middlewares/ST/STM32_USB_Device_Library/Core/Inc"
    "${BL_PLATFORM_DIR}/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc"
    "${BL_PLATFORM_DIR}/Middlewares/Third_Party/LwIP/src/include"
    "${BL_PLATFORM_DIR}/Middlewares/Third_Party/LwIP/system"
    "${BL_PLATFORM_DIR}/LWIP/App"
    "${BL_PLATFORM_DIR}/LWIP/Target"
    "${BL_PLATFORM_DIR}/Drivers/BSP/Components/lan8742"
    "${CMAKE_CURRENT_SOURCE_DIR}/user/app"
    "${CMAKE_CURRENT_SOURCE_DIR}/user/drv"
)

# MCU definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    USE_HAL_DRIVER
    STM32F746xx
    $<$<CONFIG:Debug>:DEBUG>
)

# Compiler options
target_compile_options(${PROJECT_NAME} PRIVATE
    ${CPU_FLAGS}
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
    -Wall
    -ffunction-sections
    -fdata-sections
    $<$<CONFIG:Debug>:-O0>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Release>:-Os>
)

# Linker options
target_link_options(${PROJECT_NAME} PRIVATE
    ${CPU_FLAGS}
    "--specs=nano.specs"
    "--specs=nosys.specs"
    "-T${BL_LINKER_SCRIPT}"
    "-Wl,--gc-sections"
    "-Wl,-Map=${CMAKE_CURRENT_BINARY_DIR}/bl.map"
)
