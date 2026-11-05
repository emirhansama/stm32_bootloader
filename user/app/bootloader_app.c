
/*
 * bootloader_app.c
 *
 *  Created on: Oct 14, 2026
 *      Author: emirhan.ucma
 */

#include "main.h"
#include "bootloader_app.h"
#include "bl_commands.h"
#include "bl_comm.h"
#include "bl_drv.h"
#include "bl_gpio.h"

typedef enum {
    BL_PARSE_HEADER_1 = 0,
    BL_PARSE_HEADER_2,
    BL_PARSE_LENGTH_LOW,
    BL_PARSE_LENGTH_HIGH,
    BL_PARSE_COMMAND,
    BL_PARSE_SEQUENCE,
    BL_PARSE_PAYLOAD,
    BL_PARSE_CRC
} bl_parser_state_t;

typedef struct {
    bl_parser_state_t state;
    bl_packet_t packet;
    uint16_t payload_index;
    uint8_t crc_index;
    uint32_t calculated_crc;
    uint32_t received_crc;
    uint32_t last_byte_tick;
} bl_parser_t;

static bl_parser_t parser;

/**
 * @brief Resets the packet parser state and its receive timeout.
 * @param parser Parser instance.
 */
static void bl_parser_reset(bl_parser_t *parser)
{
    parser->state = BL_PARSE_HEADER_1;
    parser->packet.length = 0;
    parser->packet.command = 0;
    parser->packet.sequence = 0;
    parser->payload_index = 0;
    parser->crc_index = 0;
    parser->calculated_crc = 0xFFFFFFFF;
    parser->received_crc = 0;
    parser->last_byte_tick = HAL_GetTick();
}

/**
 * @brief Routes a validated packet to the matching bootloader command handler.
 * @param packet Received bootloader command packet.
 */
static void bl_dispatch(const bl_packet_t *packet)
{
    switch ((BootloaderCommand_t)packet->command) {
        case BL_GET_VERSION: {
            bl_get_version_cmd(packet);
            break;
        }

        case BL_GET_HELP: {
            bl_get_help_cmd(packet);
            break;
        }

        case BL_GET_CHIP_ID: {
            bl_get_chip_id_cmd(packet);
            break;
        }

        case BL_GET_STATUS: {
            bl_get_status_cmd(packet);
            break;
        }

        case BL_START_UPDATE: {
            bl_start_update_cmd(packet);
            break;
        }

        case BL_WRITE_CHUNK: {
            bl_write_chunk_cmd(packet);
            break;
        }

        case BL_FINISH_UPDATE: {
            bl_finish_update_cmd(packet);
            break;
        }

        case BL_ABORT_UPDATE: {
            bl_abort_update_cmd(packet);
            break;
        }

        case BL_JUMP_TO_APP: {
            bl_jump_to_app_cmd(packet);
            break;
        }

        case BL_GET_RDP_STATUS: {
            bl_get_rdp_status_cmd(packet);
            break;
        }

        case BL_FLASH_ERASE: {
            bl_flash_erase_cmd(packet);
            break;
        }

        case BL_MEM_READ: {
            bl_mem_read_cmd(packet);
            break;
        }

        case BL_GET_FLASH_CRC: {
            bl_get_flash_crc_cmd(packet);
            break;
        }

        case BL_RESET: {
            bl_reset_cmd(packet);
            break;
        }

        default: {
            bl_send_response(packet->command, packet->sequence, BL_INVALID_COMMAND, NULL, 0);
            break;
        }
    }
}

/**
 * @brief Processes a received byte using the selected interface parser.
 * @param parser Parser instance.
 * @param port Communication interface.
 * @param byte Received byte.
 */
static void bl_parser_feed(bl_parser_t *parser, uint8_t byte)
{
    parser->last_byte_tick = HAL_GetTick();

    switch (parser->state) {
        case BL_PARSE_HEADER_1: {
            if (byte == BL_FRAME_HEADER_1) {
                parser->state = BL_PARSE_HEADER_2;
            }
            break;
        }

        case BL_PARSE_HEADER_2: {
            if (byte == BL_FRAME_HEADER_2) {
                parser->calculated_crc = 0xFFFFFFFF;
                parser->state = BL_PARSE_LENGTH_LOW;
            } else if (byte != BL_FRAME_HEADER_1) {
                parser->state = BL_PARSE_HEADER_1;
            }
            break;
        }

        case BL_PARSE_LENGTH_LOW: {
            parser->packet.length = byte;
            parser->calculated_crc = bl_crc32_update(parser->calculated_crc, &byte, 1);
            parser->state = BL_PARSE_LENGTH_HIGH;
            break;
        }

        case BL_PARSE_LENGTH_HIGH: {
            parser->packet.length |= (uint16_t)byte << 8;
            parser->calculated_crc = bl_crc32_update(parser->calculated_crc, &byte, 1);

            if (parser->packet.length > BL_MAX_PAYLOAD_SIZE) {
                bl_parser_reset(parser);
            } else {
                parser->state = BL_PARSE_COMMAND;
            }
            break;
        }

        case BL_PARSE_COMMAND: {
            parser->packet.command = byte;
            parser->calculated_crc = bl_crc32_update(parser->calculated_crc, &byte, 1);
            parser->state = BL_PARSE_SEQUENCE;
            break;
        }

        case BL_PARSE_SEQUENCE: {
            parser->packet.sequence = byte;
            parser->calculated_crc = bl_crc32_update(parser->calculated_crc, &byte, 1);

            parser->payload_index = 0;
            parser->crc_index = 0;
            parser->received_crc = 0;

            if (parser->packet.length == 0) {
                parser->state = BL_PARSE_CRC;
            } else {
                parser->state = BL_PARSE_PAYLOAD;
            }
            break;
        }

        case BL_PARSE_PAYLOAD: {
            parser->packet.payload[parser->payload_index] = byte;
            parser->payload_index++;

            parser->calculated_crc = bl_crc32_update(parser->calculated_crc, &byte, 1);

            if (parser->payload_index == parser->packet.length) {
                parser->state = BL_PARSE_CRC;
            }
            break;
        }

        case BL_PARSE_CRC: {
            parser->received_crc |= (uint32_t)byte << (parser->crc_index * 8);
            parser->crc_index++;

            if (parser->crc_index == 4) {
                uint32_t expected_crc = parser->calculated_crc ^ 0xFFFFFFFF;
                uint8_t command = parser->packet.command;
                uint8_t sequence = parser->packet.sequence;

                 if (expected_crc == parser->received_crc) {
                    bl_dispatch(&parser->packet);
                    bl_parser_reset(parser);
                    } else {
                        bl_parser_reset(parser);
                        bl_send_response(command, sequence, BL_CRC_ERROR, NULL, 0);
                     }
            }
            break;
        }

        default: {
            bl_parser_reset(parser);
            break;
        }
    }
}

/**
 * @brief Initializes the bootloader session and packet parsers.
 */
void bl_app_init(void)
{
    bl_gpio_init();
    bl_commands_init();
    bl_parser_reset(&parser);
}

/**
 * @brief Selects bootloader or application mode and processes incoming commands.
 */
void bl_app_run(void)
{
    uint32_t start_tick = HAL_GetTick();
    uint8_t boot_mode = 0;
    uint8_t byte;

    while ((HAL_GetTick() - start_tick) < BL_ENTRY_TIMEOUT_MS) {
        if (bl_gpio_is_boot_pressed()) {
            HAL_Delay(20);

            if (bl_gpio_is_boot_pressed()) {
                boot_mode = 1;
                break;
            }
        }
    }

    if (boot_mode == 0) {
        bl_jump_to_app();
    }

    if (bl_comm_init() != HAL_OK) {
        Error_Handler();
    }

    while (bl_comm_selected_port() == BL_COMM_NONE) {
        HAL_Delay(1);
    }

    if (bl_comm_disable_unused() != HAL_OK) {
        Error_Handler();
    }

    while (1) {
        if (bl_comm_rx_overflow() != 0) {
            bl_comm_rx_reset();
            bl_parser_reset(&parser);
        }

        if (parser.state != BL_PARSE_HEADER_1 && (HAL_GetTick() - parser.last_byte_tick) > BL_PACKET_TIMEOUT_MS) {
            bl_parser_reset(&parser);
        }

        if (bl_comm_read(&byte, 1) == 1) {
            bl_parser_feed(&parser, byte);
        } else {
            HAL_Delay(1);
        }
    }
}
