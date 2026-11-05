#include "bl_comm.h"
#include "bl_usb.h"
#include "bl_uart.h"

static volatile bl_comm_port_t active_port = BL_COMM_NONE;

/**
 * @brief Initializes USB CDC and UART communication.
 */
HAL_StatusTypeDef bl_comm_init(void)
{
    active_port = BL_COMM_NONE;

    if (bl_uart_init() != HAL_OK) {
        return HAL_ERROR;
    }

    if (bl_usb_init() != HAL_OK) {
        bl_uart_deinit();
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief Selects the first interface receiving data.
 */
uint8_t bl_comm_select_port(bl_comm_port_t port)
{
    if (port >= BL_COMM_PORT_COUNT) {
        return 0;
    }

    if (active_port == BL_COMM_NONE) {
        active_port = port;
    }

    return active_port == port;
}

/**
 * @brief Returns the selected communication interface.
 */
bl_comm_port_t bl_comm_selected_port(void)
{
    return active_port;
}

/**
 * @brief Deinitializes the unused interface.
 */
HAL_StatusTypeDef bl_comm_disable_unused(void)
{
    if (active_port == BL_COMM_USB) {
        return bl_uart_deinit();
    }

    if (active_port == BL_COMM_UART) {
        return bl_usb_deinit();
    }

    return HAL_ERROR;
}

/**
 * @brief Sends data through the active interface.
 */
HAL_StatusTypeDef bl_comm_send(const uint8_t *data, uint16_t length)
{
    switch (active_port) {
        case BL_COMM_USB:
            return bl_usb_send(data, length);

        case BL_COMM_UART:
            return bl_uart_send(data, length);

        default:
            return HAL_ERROR;
    }
}

/**
 * @brief Deinitializes the active interface.
 */
HAL_StatusTypeDef bl_comm_deinit(void)
{
    HAL_StatusTypeDef result = HAL_OK;

    if (active_port == BL_COMM_USB) {
        result = bl_usb_deinit();
    } else if (active_port == BL_COMM_UART) {
        result = bl_uart_deinit();
    }

    active_port = BL_COMM_NONE;

    return result;
}

/**
 * @brief Reads data from the active interface.
 */
uint32_t bl_comm_read(uint8_t *data, uint32_t length)
{
    switch (active_port) {
        case BL_COMM_USB:
            return bl_usb_read(data, length);

        case BL_COMM_UART:
            return bl_uart_read(data, length);

        default:
            return 0;
    }
}

/**
 * @brief Returns the active interface receive error.
 */
uint8_t bl_comm_rx_overflow(void)
{
    switch (active_port) {
        case BL_COMM_USB:
            return bl_usb_rx_overflow();

        case BL_COMM_UART:
            return bl_uart_rx_overflow();

        default:
            return 0;
    }
}

/**
 * @brief Resets the active interface receive buffer.
 */
void bl_comm_rx_reset(void)
{
    switch (active_port) {
        case BL_COMM_USB:
            bl_usb_rx_reset();
            break;

        case BL_COMM_UART:
            bl_uart_rx_reset();
            break;

        default:
            break;
    }
}