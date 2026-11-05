/*
 * bl_usb.c
 *
 *  Created on: Oct 20, 2026
 *      Author: emirhan.ucma
 */

#include "bl_usb.h"
#include "ring_buffer.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "bl_comm.h"

#include <string.h>

RING_BUF_DECLARE(usb_rx_ring, BL_USB_RX_QUEUE_SIZE);

static volatile uint8_t rx_overflow = 0;
static uint8_t usb_initialized = 0;
static uint8_t usb_tx_buffer[BL_USB_TX_SIZE];

/**
 * @brief Queues incoming USB CDC bytes and detects receive overflow.
 * @param data Bytes delivered by the USB CDC receive callback.
 * @param length Number of received bytes.
 */
void bl_usb_on_receive(const uint8_t *data, uint32_t length)
{
    if (data == NULL || length == 0 || rx_overflow != 0) {
        return;
    }

    if (bl_comm_select_port(BL_COMM_USB) == 0) {
        return;
    }

    if (length > ring_buf_space_get(&usb_rx_ring)) {
        rx_overflow = 1;
        return;
    }

    if (ring_buf_put(&usb_rx_ring, data, length) != length) {
        rx_overflow = 1;
    }
}

/**
 * @brief Reads queued bytes from the USB CDC receive ring buffer.
 * @param data Data buffer.
 * @param length Number of bytes.
 * @return Number of bytes read.
 */
uint32_t bl_usb_read(uint8_t *data, uint32_t length)
{
    return ring_buf_get(&usb_rx_ring, data, length);
}

/**
 * @brief Returns the latched USB CDC receive overflow flag.
 * @return Nonzero if receive overflow occurred.
 */
uint8_t bl_usb_rx_overflow(void)
{
    return rx_overflow;
}

/**
 * @brief Resets the USB CDC receive queue and overflow flag.
 */
void bl_usb_rx_reset(void)
{
    ring_buf_reset(&usb_rx_ring);
    rx_overflow = 0;
}

/**
 * @brief Transmits a USB CDC response and waits for TX completion.
 * @param data Response bytes to transmit.
 * @param length Number of bytes.
 * @retval HAL_StatusTypeDef HAL status.
 */
HAL_StatusTypeDef bl_usb_send(const uint8_t *data, uint16_t length)
{
    if (data == NULL || length == 0 || length > BL_USB_TX_SIZE) {
        return HAL_ERROR;
    }

    uint32_t start_tick = HAL_GetTick();

    while (CDC_TxComplete_FS() == 0) {
        if ((HAL_GetTick() - start_tick) >= BL_USB_TIMEOUT_MS) {
            return HAL_TIMEOUT;
        }
    }

    memcpy(usb_tx_buffer, data, length);

    uint8_t result = CDC_Transmit_FS(usb_tx_buffer, length);
    if (result == USBD_BUSY) {
        return HAL_BUSY;
    }
    if (result != USBD_OK) {
        return HAL_ERROR;
    }

    while (CDC_TxComplete_FS() == 0) {
        if ((HAL_GetTick() - start_tick) >= BL_USB_TIMEOUT_MS) {
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

/**
 * @brief Stops and deinitializes USB CDC if it was initialized.
 * @retval HAL_StatusTypeDef HAL status.
 */
HAL_StatusTypeDef bl_usb_deinit(void)
{
    if (usb_initialized == 0) {
        return HAL_OK;
    }

    USBD_Stop(&hUsbDeviceFS);
    if (USBD_DeInit(&hUsbDeviceFS) != USBD_OK) {
        return HAL_ERROR;
    }

    usb_initialized = 0;
    return HAL_OK;
}

/**
 * @brief Initializes the USB CDC interface if it is not already active.
 * @retval HAL_StatusTypeDef HAL status.
 */
HAL_StatusTypeDef bl_usb_init(void)
{
    if (usb_initialized != 0) {
        return HAL_OK;
    }

    bl_usb_rx_reset();
    MX_USB_DEVICE_Init();
    usb_initialized = 1;
    return HAL_OK;
}

