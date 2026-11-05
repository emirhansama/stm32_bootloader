/*
 * bl_uart.c
 *
 * UART communication and GPIO/clock/IRQ initialization are owned by
 * the bootloader. The selected pin and alternate-function mapping is
 * provided by platform/<mcu>/bl_uart_pinmap.h at build time.
 */

#include "bl_uart.h"
#include "bl_uart_pinmap.h"
#include "ring_buffer.h"
#include "bl_comm.h"

RING_BUF_DECLARE(uart_rx_ring, BL_UART_RX_QUEUE_SIZE);

static UART_HandleTypeDef uart_handle;
static uint8_t uart_rx_byte;
static volatile uint8_t uart_rx_error = 0;
static uint8_t uart_initialized = 0;

/**
 * @brief Enables GPIO clocks, sets pin alternate functions and starts the UART.
 */
static HAL_StatusTypeDef bl_uart_hw_init(void)
{
    BL_UART_TX_GPIO_CLK_ENABLE();
    BL_UART_RX_GPIO_CLK_ENABLE();
    BL_UART_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    gpio.Pin = BL_UART_TX_GPIO_PIN;
    gpio.Alternate = BL_UART_TX_AF;
    HAL_GPIO_Init(BL_UART_TX_GPIO, &gpio);

    gpio.Pin = BL_UART_RX_GPIO_PIN;
    gpio.Alternate = BL_UART_RX_AF;
    HAL_GPIO_Init(BL_UART_RX_GPIO, &gpio);

    uart_handle.Instance = BL_UART_INSTANCE;
    uart_handle.Init.BaudRate = BL_UART_BAUDRATE;
    uart_handle.Init.WordLength = UART_WORDLENGTH_8B;
    uart_handle.Init.StopBits = UART_STOPBITS_1;
    uart_handle.Init.Parity = UART_PARITY_NONE;
    uart_handle.Init.Mode = UART_MODE_TX_RX;
    uart_handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart_handle.Init.OverSampling = UART_OVERSAMPLING_16;

    return HAL_UART_Init(&uart_handle);
}

/**
 * @brief Initializes the selected UART and starts interrupt reception.
 */
HAL_StatusTypeDef bl_uart_init(void)
{
    if (uart_initialized != 0) {
        return HAL_OK;
    }

    bl_uart_rx_reset();

    if (bl_uart_hw_init() != HAL_OK) {
        HAL_UART_DeInit(&uart_handle);
        HAL_GPIO_DeInit(BL_UART_TX_GPIO, BL_UART_TX_GPIO_PIN);
        HAL_GPIO_DeInit(BL_UART_RX_GPIO, BL_UART_RX_GPIO_PIN);
        BL_UART_CLK_DISABLE();
        return HAL_ERROR;
    }

    HAL_NVIC_SetPriority(BL_UART_IRQN, 6, 0);
    HAL_NVIC_EnableIRQ(BL_UART_IRQN);
    uart_initialized = 1;

    if (HAL_UART_Receive_IT(&uart_handle, &uart_rx_byte, 1) != HAL_OK) {
        bl_uart_deinit();
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief Sends data through the selected UART.
 */
HAL_StatusTypeDef bl_uart_send(const uint8_t *data, uint16_t length)
{
    if (uart_initialized == 0 || data == NULL || length == 0) {
        return HAL_ERROR;
    }

    return HAL_UART_Transmit(&uart_handle, (uint8_t *)data, length, BL_UART_TX_TIMEOUT_MS);
}

/**
 * @brief Reads received bytes from the ring buffer.
 */
uint32_t bl_uart_read(uint8_t *data, uint32_t length)
{
    if (uart_initialized == 0 || data == NULL || length == 0) {
        return 0;
    }

    return ring_buf_get(&uart_rx_ring, data, length);
}

uint8_t bl_uart_rx_overflow(void)
{
    return uart_rx_error;
}

void bl_uart_rx_reset(void)
{
    ring_buf_reset(&uart_rx_ring);
    uart_rx_error = 0;
}

/**
 * @brief Stops the UART, disables its interrupt and releases the selected pins.
 */
HAL_StatusTypeDef bl_uart_deinit(void)
{
    if (uart_initialized == 0) {
        return HAL_OK;
    }

    HAL_NVIC_DisableIRQ(BL_UART_IRQN);
    HAL_UART_AbortReceive(&uart_handle);
    HAL_StatusTypeDef status = HAL_UART_DeInit(&uart_handle);

    HAL_GPIO_DeInit(BL_UART_TX_GPIO, BL_UART_TX_GPIO_PIN);
    HAL_GPIO_DeInit(BL_UART_RX_GPIO, BL_UART_RX_GPIO_PIN);
    BL_UART_CLK_DISABLE();

    uart_initialized = 0;
    return status;
}

/**
 * @brief UART RX interrupt callback; preserves first-arriving-port selection.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != BL_UART_INSTANCE || uart_initialized == 0) {
        return;
    }

    if (bl_comm_select_port(BL_COMM_UART) == 0) {
        return;
    }

    if (uart_rx_error == 0 && ring_buf_put(&uart_rx_ring, &uart_rx_byte, 1) != 1) {
        uart_rx_error = 1;
    }

    if (HAL_UART_Receive_IT(&uart_handle, &uart_rx_byte, 1) != HAL_OK) {
        uart_rx_error = 1;
    }
}

/**
 * @brief UART error callback.
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != BL_UART_INSTANCE || uart_initialized == 0) {
        return;
    }

    uart_rx_error = 1;
    __HAL_UART_CLEAR_OREFLAG(&uart_handle);
    HAL_UART_Receive_IT(&uart_handle, &uart_rx_byte, 1);
}

/**
 * @brief IRQ vector for the configured UART (generated pinmap selects its name).
 */
void BL_UART_ISR(void)
{
    HAL_UART_IRQHandler(&uart_handle);
}
