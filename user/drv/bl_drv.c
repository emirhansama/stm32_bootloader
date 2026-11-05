
#include "bl_drv.h"
#include "bootloader_app.h"
#include "bl_comm.h"
#include "bl_platform.h"

/**
 * @brief Reads a little-endian 16-bit integer.
 */
uint16_t bl_read_u16(const uint8_t *data)
{
    return (uint16_t)data[0] | ((uint16_t)data[1] << 8);
}

/**
 * @brief Reads a little-endian 32-bit integer.
 */
uint32_t bl_read_u32(const uint8_t *data)
{
    return (uint32_t)data[0] |
           ((uint32_t)data[1] << 8) |
           ((uint32_t)data[2] << 16) |
           ((uint32_t)data[3] << 24);
}

/**
 * @brief Writes a little-endian 32-bit integer.
 */
void bl_write_u32(uint8_t *data, uint32_t value)
{
    data[0] = (uint8_t)value;
    data[1] = (uint8_t)(value >> 8);
    data[2] = (uint8_t)(value >> 16);
    data[3] = (uint8_t)(value >> 24);
}

/**
 * @brief Updates an existing CRC32 accumulator.
 */
uint32_t bl_crc32_update(uint32_t crc, const uint8_t *data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (uint32_t bit = 0; bit < 8; bit++) {
            if ((crc & 1) != 0) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc;
}

/**
 * @brief Calculates CRC32 for a buffer.
 */
uint32_t bl_crc32(const uint8_t *data, uint32_t length)
{
    return bl_crc32_update(0xFFFFFFFF, data, length) ^ 0xFFFFFFFF;
}

/**
 * @brief Validates the application stack pointer and reset handler.
 */
uint8_t bl_app_vectors_valid(uint32_t msp, uint32_t reset, uint32_t image_end)
{
    uint32_t reset_address = reset & ~1;

    if (msp <= BL_APP_SRAM_START || msp > BL_APP_SRAM_END || (msp & 7) != 0 ||
        (reset & 1) == 0 || reset_address < BL_APP_START_ADDRESS ||
        image_end > BL_FLASH_END_ADDRESS || reset_address >= image_end) {
        return 0;
    }

    return 1;
}

/**
 * @brief Switches MSP and branches to the application reset handler.
 */
__attribute__((naked, noreturn)) static void bl_branch_to_application(uint32_t msp, uint32_t reset)
{
    __asm volatile (
        "msr msp, r0\n"
        "cpsie i\n"
        "bx r1\n"
    );
}

/**
 * @brief Deinitializes bootloader state and jumps to the application.
 * @note Successful jump does not return.
 */
HAL_StatusTypeDef bl_jump_to_app(void)
{
    uint32_t msp = *(volatile const uint32_t *)BL_APP_START_ADDRESS;
    uint32_t reset = *(volatile const uint32_t *)(BL_APP_START_ADDRESS + 4);

    if (bl_app_vectors_valid(msp, reset, BL_FLASH_END_ADDRESS) == 0) {
        return HAL_ERROR;
    }

    if (bl_comm_deinit() != HAL_OK) {
        return HAL_ERROR;
    }

    if (HAL_DeInit() != HAL_OK || HAL_RCC_DeInit() != HAL_OK) {
        return HAL_ERROR;
    }

    __disable_irq();

    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

#if defined(STM32F746xx)
    if ((SCB->CCR & SCB_CCR_DC_Msk) != 0) {
        SCB_DisableDCache();
    }

    if ((SCB->CCR & SCB_CCR_IC_Msk) != 0) {
        SCB_DisableICache();
    }

    HAL_MPU_Disable();
#endif

    for (uint32_t i = 0; i < sizeof(NVIC->ICER) / sizeof(NVIC->ICER[0]); i++) {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk | SCB_ICSR_PENDSVCLR_Msk;
    SCB->VTOR = BL_APP_START_ADDRESS;

    __set_CONTROL(0);

    __DSB();
    __ISB();

    bl_branch_to_application(msp, reset);
}
