/*
 * bl_usb.h
 *
 *  Created on: Oct 20, 2026
 *      Author: emirhan.ucma
 */

#ifndef DRV_BL_USB_H_
#define DRV_BL_USB_H_

#include "main.h"

#define BL_USB_TIMEOUT_MS 1000
#define BL_USB_TX_SIZE 512
#define BL_USB_RX_QUEUE_SIZE 1024

/**
 * @brief Initializes the USB CDC interface if it is not already active.
 * @retval HAL_StatusTypeDef HAL status.
 */
HAL_StatusTypeDef bl_usb_init(void);
/**
 * @brief Transmits a USB CDC response and waits for TX completion.
 * @param data Response bytes to transmit.
 * @param length Number of bytes.
 * @retval HAL_StatusTypeDef HAL status.
 */
HAL_StatusTypeDef bl_usb_send(const uint8_t *data, uint16_t length);
/**
 * @brief Stops and deinitializes USB CDC if it was initialized.
 * @retval HAL_StatusTypeDef HAL status.
 */
HAL_StatusTypeDef bl_usb_deinit(void);
/**
 * @brief Reads queued bytes from the USB CDC receive ring buffer.
 * @param data Data buffer.
 * @param length Number of bytes.
 * @return Number of bytes read.
 */
uint32_t bl_usb_read(uint8_t *data, uint32_t length);
/**
 * @brief Returns the latched USB CDC receive overflow flag.
 * @return Nonzero if receive overflow occurred.
 */
uint8_t bl_usb_rx_overflow(void);
/**
 * @brief Resets the USB CDC receive queue and overflow flag.
 */
void bl_usb_rx_reset(void);
/**
 * @brief Queues incoming USB CDC bytes and detects receive overflow.
 * @param data Bytes delivered by the USB CDC receive callback.
 * @param length Number of received bytes.
 */
void bl_usb_on_receive(const uint8_t *data, uint32_t length);

#endif /* DRV_BL_USB_H_ */
