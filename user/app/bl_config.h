#ifndef BL_CONFIG_H
#define BL_CONFIG_H

#define BL_MCU_STM32F407 1
#define BL_MCU_STM32F746 2

#define BL_MCU BL_MCU_STM32F407

// configure uart pins acoording to your mcu. bootloader_app has is own control for inaccurate board/uart/pin matchings
#include "bl_gpio_ports.h"
#define BL_UART_ID 3
#define BL_UART_BAUDRATE 115200
#define BL_UART_TX_PORT BL_PORT_D
#define BL_UART_TX_PIN 8
#define BL_UART_RX_PORT BL_PORT_D
#define BL_UART_RX_PIN 9

// Boot button: port, pin number (0-15), pressed level (0/1), pull mode.
#define BL_BOOT_PORT BL_PORT_A
#define BL_BOOT_PIN 0
#define BL_BOOT_ACTIVE_LEVEL 1
#define BL_BOOT_PULL GPIO_NOPULL

#endif /* BL_CONFIG_H */