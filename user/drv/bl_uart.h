#ifndef BL_UART_H
#define BL_UART_H

#include "main.h"

#define BL_UART_RX_QUEUE_SIZE 1024
#define BL_UART_TX_TIMEOUT_MS 1000

/**
* @brief Initializes the selected UART interface for bootloader communication.
* @retval HAL_StatusTypeDef HAL status.
*/
HAL_StatusTypeDef bl_uart_init(void);

/**
* @brief sends response data through the selected UART interface.
* @param data Pointer to the data to be sent.
* @param length Size of the data to be sent.
* @retval HAL_StatusTypeDef HAL status.
*/
HAL_StatusTypeDef bl_uart_send(const uint8_t *data, uint16_t length);

/**
* @brief Reads data from the UART receive buffer.
* @param data Pointer to the buffer where received data will be stored.
* @param length Maximum number of bytes to read.
* @return Number of bytes read.
*/
uint32_t bl_uart_read(uint8_t *data, uint32_t length);

/**
* @brief Checks if the UART receive buffer has overflowed.
* @retval non-zero if overflow occurred, 0 otherwise.
*/
uint8_t bl_uart_rx_overflow(void);

/**
* @brief Resets the UART receive buffer and clear error flag.
*/
void bl_uart_rx_reset(void);

/**
* @brief Deinitializes the selected UART interface.
* @retval HAL_StatusTypeDef HAL status.
*/
HAL_StatusTypeDef bl_uart_deinit(void);

#endif /* BL_UART_H */