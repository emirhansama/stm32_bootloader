
#ifndef BL_DRV_H
#define BL_DRV_H

#include "main.h"

uint16_t bl_read_u16(const uint8_t *data);
uint32_t bl_read_u32(const uint8_t *data);
void bl_write_u32(uint8_t *data, uint32_t value);

uint32_t bl_crc32_update(uint32_t crc, const uint8_t *data, uint32_t length);
uint32_t bl_crc32(const uint8_t *data, uint32_t length);

/**
 * @brief Validates application stack and reset vectors.
 */
uint8_t bl_app_vectors_valid(uint32_t msp, uint32_t reset, uint32_t image_end);

/**
 * @brief Deinitializes peripherals and jumps to the application.
 * @note Does not return on a successful jump.
 */
HAL_StatusTypeDef bl_jump_to_app(void);

#endif /* BL_DRV_H */
