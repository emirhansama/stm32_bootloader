/*
 * bl_commands.c
 *
 *  Created on: Oct 14, 2026
 *      Author: emirhan.ucma
 */
#include "main.h"
#include "bl_commands.h"
#include "bootloader_app.h"
#include "bl_comm.h"
#include "bl_drv.h"
#include "bl_platform.h"

#include <string.h>

_Static_assert((BL_FLASH_APP_FIRST_SECTOR + BL_FLASH_APP_SECTOR_COUNT) <= FLASH_SECTOR_TOTAL, "Invalid application flash sector range");
_Static_assert((BL_FLASH_APP_FIRST_SECTOR_SIZE + (BL_FLASH_APP_SECTOR_COUNT - 1) * BL_FLASH_APP_NEXT_SECTOR_SIZE) == (BL_FLASH_END_ADDRESS - BL_APP_START_ADDRESS), "Application flash layout mismatch");

static BootloaderState_t bootloader_state = BL_STATE_IDLE;

typedef struct {
    uint32_t firmware_size;
    uint32_t firmware_crc;
    uint32_t written_size;
    uint8_t initial_vectors[8];
    uint8_t active;
    uint8_t flash_ready;
} bl_update_session_t;

static bl_update_session_t update_session = {0};

void bl_commands_init(void)
{
    memset(&update_session, 0, sizeof(update_session));
    bootloader_state = BL_STATE_IDLE;
}

HAL_StatusTypeDef bl_send_response(uint8_t command, uint8_t sequence, BootloaderStatus_t status, const uint8_t *payload, uint16_t length)
{
    uint8_t frame[BL_FRAME_MAX_SIZE];

    if (length > BL_MAX_PAYLOAD_SIZE - 1 || (length != 0 && payload == NULL)) {
        return HAL_ERROR;
    }

    uint16_t payload_length = length + 1;

    frame[0] = BL_FRAME_HEADER_1;
    frame[1] = BL_FRAME_HEADER_2;
    frame[2] = (uint8_t)payload_length;
    frame[3] = (uint8_t)(payload_length >> 8);
    frame[4] = command;
    frame[5] = sequence;
    frame[6] = (uint8_t)status;

    if (length != 0) {
        memcpy(&frame[7], payload, length);
    }

    uint32_t crc = bl_crc32(&frame[2], (uint32_t)payload_length + 4);
    uint16_t crc_position = (uint16_t)(6 + payload_length);

    bl_write_u32(&frame[crc_position], crc);

    return bl_comm_send(frame, (uint16_t)(crc_position + 4));
}

static HAL_StatusTypeDef bl_send_status(const bl_packet_t *packet, BootloaderStatus_t status)
{
    return bl_send_response(packet->command, packet->sequence, status, NULL, 0);
}

static void bl_flash_failure(void)
{
    update_session.flash_ready = 0;
    bootloader_state = BL_STATE_ERROR;
}

HAL_StatusTypeDef bl_get_version_cmd(const bl_packet_t *packet)
{
    if (packet->length != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    static const char version[] = "Bootloader Version: 1.0";

    return bl_send_response(packet->command, packet->sequence, BL_OK, (const uint8_t *)version, sizeof(version) - 1);
}

HAL_StatusTypeDef bl_get_help_cmd(const bl_packet_t *packet)
{
    if (packet->length != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    const uint8_t supported[] = {
        BL_GET_VERSION,
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
    };

    return bl_send_response(packet->command, packet->sequence, BL_OK, supported, sizeof(supported));
}

HAL_StatusTypeDef bl_get_chip_id_cmd(const bl_packet_t *packet)
{
    if (packet->length != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    uint32_t id = HAL_GetDEVID();

    uint8_t data[2] = {
        (uint8_t)id,
        (uint8_t)(id >> 8)
    };

    return bl_send_response(packet->command, packet->sequence, BL_OK, data, sizeof(data));
}

HAL_StatusTypeDef bl_get_status_cmd(const bl_packet_t *packet)
{
    if (packet->length != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    uint8_t state = (uint8_t)bootloader_state;

    return bl_send_response(packet->command, packet->sequence, BL_OK, &state, 1);
}

HAL_StatusTypeDef bl_get_rdp_status_cmd(const bl_packet_t *packet)
{
    if (packet->length != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    FLASH_OBProgramInitTypeDef options = {0};

    HAL_FLASHEx_OBGetConfig(&options);

    uint8_t level = (uint8_t)options.RDPLevel;

    return bl_send_response(packet->command, packet->sequence, BL_OK, &level, 1);
}

HAL_StatusTypeDef bl_start_update_cmd(const bl_packet_t *packet)
{
    if (packet->length != 8) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    if (update_session.active != 0) {
        return bl_send_status(packet, BL_BUSY);
    }

    uint32_t size = bl_read_u32(&packet->payload[0]);
    uint32_t crc = bl_read_u32(&packet->payload[4]);
    uint32_t capacity = BL_FLASH_END_ADDRESS - BL_APP_START_ADDRESS;

    if (size < 8 || size > capacity) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    memset(&update_session, 0, sizeof(update_session));

    update_session.firmware_size = size;
    update_session.firmware_crc = crc;
    update_session.active = 1;

    bootloader_state = BL_STATE_IDLE;

    return bl_send_status(packet, BL_OK);
}

/**
 * @brief Erases only the application sectors required for the firmware.
 */
HAL_StatusTypeDef bl_flash_erase_cmd(const bl_packet_t *packet)
{
    if (packet->length != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    if (update_session.active == 0 || update_session.flash_ready != 0 || update_session.written_size != 0 || bootloader_state == BL_STATE_ERROR) {
        return bl_send_status(packet, BL_INVALID_STATE);
    }

    uint32_t sector_count = 1;

    if (update_session.firmware_size > BL_FLASH_APP_FIRST_SECTOR_SIZE) {
        uint32_t remaining = update_session.firmware_size - BL_FLASH_APP_FIRST_SECTOR_SIZE;
        sector_count += (remaining + BL_FLASH_APP_NEXT_SECTOR_SIZE - 1) / BL_FLASH_APP_NEXT_SECTOR_SIZE;
    }

    if (sector_count == 0 || sector_count > BL_FLASH_APP_SECTOR_COUNT) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    uint32_t sector_error = 0xFFFFFFFF;

    FLASH_EraseInitTypeDef erase = {0};

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector = BL_FLASH_APP_FIRST_SECTOR;
    erase.NbSectors = sector_count;

    bootloader_state = BL_STATE_ERASING;

    if (HAL_FLASH_Unlock() != HAL_OK) {
        bl_flash_failure();
        return bl_send_status(packet, BL_FLASH_ERROR);
    }

    __HAL_FLASH_CLEAR_FLAG(BL_FLASH_CLEAR_FLAGS);

    HAL_StatusTypeDef result = HAL_FLASHEx_Erase(&erase, &sector_error);
    HAL_StatusTypeDef lock_result = HAL_FLASH_Lock();

    if (result != HAL_OK || lock_result != HAL_OK ||
        *(volatile const uint32_t *)BL_APP_START_ADDRESS != 0xFFFFFFFF ||
        *(volatile const uint32_t *)(BL_APP_START_ADDRESS + 4) != 0xFFFFFFFF) {
        bl_flash_failure();
        return bl_send_status(packet, BL_FLASH_ERROR);
    }

    update_session.flash_ready = 1;
    bootloader_state = BL_STATE_RECEIVING;

    return bl_send_status(packet, BL_OK);
}

HAL_StatusTypeDef bl_write_chunk_cmd(const bl_packet_t *packet)
{
    if (packet->length <= 4 || packet->length > BL_MAX_PAYLOAD_SIZE) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    if (update_session.active == 0 || update_session.flash_ready == 0 || bootloader_state != BL_STATE_RECEIVING) {
        return bl_send_status(packet, BL_INVALID_STATE);
    }

    uint32_t offset = bl_read_u32(&packet->payload[0]);
    uint32_t data_size = (uint32_t)packet->length - 4;
    const uint8_t *data = &packet->payload[4];

    if (offset != update_session.written_size) {
        return bl_send_status(packet, BL_SEQUENCE_ERROR);
    }

    if (offset >= update_session.firmware_size || data_size > update_session.firmware_size - offset || (offset & 3) != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    if (offset + data_size != update_session.firmware_size && (data_size & 3) != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    if (HAL_FLASH_Unlock() != HAL_OK) {
        bl_flash_failure();
        return bl_send_status(packet, BL_FLASH_ERROR);
    }

    __HAL_FLASH_CLEAR_FLAG(BL_FLASH_CLEAR_FLAGS);

    uint32_t i = 0;
    HAL_StatusTypeDef write_result = HAL_OK;

    while (i < data_size) {
        uint32_t position = offset + i;

        if (position < sizeof(update_session.initial_vectors)) {
            update_session.initial_vectors[position] = data[i];
            i++;
            continue;
        }

        uint32_t address = BL_APP_START_ADDRESS + position;

        if ((data_size - i) >= 4) {
            uint32_t value = bl_read_u32(&data[i]);

            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, value) != HAL_OK) {
                write_result = HAL_ERROR;
                break;
            }

            i += 4;
        } else {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address, data[i]) != HAL_OK) {
                write_result = HAL_ERROR;
                break;
            }

            i++;
        }
    }

    HAL_StatusTypeDef lock_result = HAL_FLASH_Lock();

    if (write_result != HAL_OK || lock_result != HAL_OK) {
        bl_flash_failure();
        return bl_send_status(packet, BL_FLASH_ERROR);
    }

    update_session.written_size += data_size;

    uint8_t progress[4];
    bl_write_u32(progress, update_session.written_size);

    return bl_send_response(packet->command, packet->sequence, BL_OK, progress, sizeof(progress));
}

/**
 * @brief Verifies firmware CRC and programs the initial vectors last.
 */
HAL_StatusTypeDef bl_finish_update_cmd(const bl_packet_t *packet)
{
    if (packet->length != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    if (update_session.active == 0 || update_session.flash_ready == 0 || bootloader_state != BL_STATE_RECEIVING) {
        return bl_send_status(packet, BL_INVALID_STATE);
    }

    if (update_session.written_size != update_session.firmware_size) {
        return bl_send_status(packet, BL_INCOMPLETE_UPDATE);
    }

    bootloader_state = BL_STATE_VERIFYING;

    uint32_t msp = bl_read_u32(&update_session.initial_vectors[0]);
    uint32_t reset = bl_read_u32(&update_session.initial_vectors[4]);

    if (bl_app_vectors_valid(msp, reset, BL_APP_START_ADDRESS + update_session.firmware_size) == 0) {
        bl_flash_failure();
        return bl_send_status(packet, BL_INVALID_APPLICATION);
    }

    uint32_t crc = bl_crc32_update(0xFFFFFFFF, update_session.initial_vectors, 8);

    crc = bl_crc32_update(crc, (const uint8_t *)(BL_APP_START_ADDRESS + 8), update_session.firmware_size - 8);
    crc ^= 0xFFFFFFFF;

    if (crc != update_session.firmware_crc) {
        bl_flash_failure();
        return bl_send_status(packet, BL_CRC_ERROR);
    }

    if (HAL_FLASH_Unlock() != HAL_OK) {
        bl_flash_failure();
        return bl_send_status(packet, BL_FLASH_ERROR);
    }

    __HAL_FLASH_CLEAR_FLAG(BL_FLASH_CLEAR_FLAGS);

    HAL_StatusTypeDef result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, BL_APP_START_ADDRESS, msp);

    if (result == HAL_OK) {
        result = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, BL_APP_START_ADDRESS + 4, reset);
    }

    HAL_StatusTypeDef lock_result = HAL_FLASH_Lock();

    if (result != HAL_OK || lock_result != HAL_OK ||
        *(volatile const uint32_t *)BL_APP_START_ADDRESS != msp ||
        *(volatile const uint32_t *)(BL_APP_START_ADDRESS + 4) != reset) {
        bl_flash_failure();
        return bl_send_status(packet, BL_FLASH_ERROR);
    }

    memset(&update_session, 0, sizeof(update_session));
    bootloader_state = BL_STATE_READY;

    return bl_send_status(packet, BL_OK);
}

HAL_StatusTypeDef bl_abort_update_cmd(const bl_packet_t *packet)
{
    if (packet->length != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    if (update_session.active == 0) {
        return bl_send_status(packet, BL_INVALID_STATE);
    }

    memset(&update_session, 0, sizeof(update_session));
    bootloader_state = BL_STATE_IDLE;

    return bl_send_status(packet, BL_OK);
}

HAL_StatusTypeDef bl_mem_read_cmd(const bl_packet_t *packet)
{
    if (packet->length != 6) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    uint32_t address = bl_read_u32(&packet->payload[0]);
    uint16_t length = bl_read_u16(&packet->payload[4]);

    if (length == 0 || length > BL_MAX_PAYLOAD_SIZE - 1) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    if (length == 0 || address < BL_APP_START_ADDRESS || address >= BL_FLASH_END_ADDRESS ||
        length > BL_FLASH_END_ADDRESS - address) {
        return bl_send_status(packet, BL_INVALID_ADDRESS);
    }

    return bl_send_response(packet->command, packet->sequence, BL_OK, (const uint8_t *)address, length);
}

HAL_StatusTypeDef bl_get_flash_crc_cmd(const bl_packet_t *packet)
{
    if (packet->length != 8) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    uint32_t address = bl_read_u32(&packet->payload[0]);
    uint32_t length = bl_read_u32(&packet->payload[4]);

    if (length == 0 || address < BL_APP_START_ADDRESS || address >= BL_FLASH_END_ADDRESS ||
        length > BL_FLASH_END_ADDRESS - address) {
        return bl_send_status(packet, BL_INVALID_ADDRESS);
    }

    uint32_t crc = bl_crc32((const uint8_t *)address, length);

    uint8_t response[4];
    bl_write_u32(response, crc);

    return bl_send_response(packet->command, packet->sequence, BL_OK, response, sizeof(response));
}

HAL_StatusTypeDef bl_jump_to_app_cmd(const bl_packet_t *packet)
{
    if (packet->length != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    if (update_session.active != 0) {
        return bl_send_status(packet, BL_INVALID_STATE);
    }

    uint32_t msp = *(volatile const uint32_t *)BL_APP_START_ADDRESS;
    uint32_t reset = *(volatile const uint32_t *)(BL_APP_START_ADDRESS + 4);

    if (bl_app_vectors_valid(msp, reset, BL_FLASH_END_ADDRESS) == 0) {
        return bl_send_status(packet, BL_INVALID_APPLICATION);
    }

    if (bl_send_status(packet, BL_OK) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_Delay(20);

    return bl_jump_to_app();
}

HAL_StatusTypeDef bl_reset_cmd(const bl_packet_t *packet)
{
    if (packet->length != 0) {
        return bl_send_status(packet, BL_INVALID_LENGTH);
    }

    if (bl_send_status(packet, BL_OK) != HAL_OK) {
        return HAL_ERROR;
    }

    HAL_Delay(20);

    NVIC_SystemReset();

    return HAL_OK;
}

