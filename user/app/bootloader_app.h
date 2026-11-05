/*
 * bootloader_app.h
 *
 *  Created on: Oct 14, 2026
 *      Author: emirhan.ucma
 */


#ifndef BOOTLOADER_APP_H
#define BOOTLOADER_APP_H

#include "main.h"

#define BL_APP_START_ADDRESS    0x08020000
#define BL_FLASH_END_ADDRESS    0x08100000
#define BL_ENTRY_TIMEOUT_MS     3000
#define BL_PACKET_TIMEOUT_MS    500

/**
 * @brief Initializes the bootloader session and packet parser.
 */
void bl_app_init(void);
/**
 * @brief Selects bootloader or application mode and processes incoming commands.
 */
void bl_app_run(void);

#endif /* BOOTLOADER_APP_H */
