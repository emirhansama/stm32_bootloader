/*
 * bl_commands.h
 *
 *  Created on: Oct 14, 2026
 *      Author: emirhan.ucma
 */



#ifndef BL_COMMANDS_H
#define BL_COMMANDS_H

#include "main.h"

#define BL_MAX_PAYLOAD_SIZE 256
#define BL_FRAME_MAX_SIZE (10 + BL_MAX_PAYLOAD_SIZE)
#define BL_CHUNK_DATA_SIZE (BL_MAX_PAYLOAD_SIZE - 4)
#define BL_FRAME_HEADER_1 0xAA
#define BL_FRAME_HEADER_2 0x55

typedef enum {
    BL_STATE_IDLE = 0,
    BL_STATE_ERASING,
    BL_STATE_RECEIVING,
    BL_STATE_VERIFYING,
    BL_STATE_READY,
    BL_STATE_ERROR
} BootloaderState_t;

typedef enum {
    BL_GET_VERSION = 1,
    BL_GET_HELP,
    BL_GET_CHIP_ID,
    BL_GET_STATUS,
    BL_START_UPDATE,
    BL_WRITE_CHUNK,
    BL_FINISH_UPDATE,
    BL_ABORT_UPDATE,
    BL_JUMP_TO_APP,
    BL_GET_RDP_STATUS,
    BL_FLASH_ERASE,
    BL_MEM_READ,
    BL_GET_FLASH_CRC,
    BL_RESET
} BootloaderCommand_t;

typedef enum {
    BL_OK = 0,
    BL_ERROR,
    BL_BUSY,
    BL_TIMEOUT,
    BL_INVALID_COMMAND,
    BL_INVALID_ADDRESS,
    BL_INVALID_LENGTH,
    BL_CRC_ERROR,
    BL_FLASH_ERROR,
    BL_UART_ERROR,
    BL_INVALID_APPLICATION,
    BL_INVALID_STATE,
    BL_SEQUENCE_ERROR,
    BL_INCOMPLETE_UPDATE
} BootloaderStatus_t;

typedef struct {
    uint16_t length;
    uint8_t command;
    uint8_t sequence;
    uint8_t payload[BL_MAX_PAYLOAD_SIZE];
} bl_packet_t;

void bl_commands_init(void);
HAL_StatusTypeDef bl_send_response(uint8_t command, uint8_t sequence, BootloaderStatus_t status, const uint8_t *payload, uint16_t length);
HAL_StatusTypeDef bl_get_version_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_get_help_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_get_chip_id_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_get_status_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_start_update_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_write_chunk_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_finish_update_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_abort_update_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_jump_to_app_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_get_rdp_status_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_flash_erase_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_mem_read_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_get_flash_crc_cmd(const bl_packet_t *packet);
HAL_StatusTypeDef bl_reset_cmd(const bl_packet_t *packet);

#endif /* BL_COMMANDS_H */
