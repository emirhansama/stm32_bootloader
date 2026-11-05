#ifndef DRV_BL_COMM_H_
#define DRV_BL_COMM_H_

#include "main.h"

typedef enum {
    BL_COMM_USB = 0,
    BL_COMM_UART,
    BL_COMM_PORT_COUNT,
    BL_COMM_NONE = BL_COMM_PORT_COUNT
} bl_comm_port_t;

HAL_StatusTypeDef bl_comm_init(void);
HAL_StatusTypeDef bl_comm_send(const uint8_t *data, uint16_t length);
HAL_StatusTypeDef bl_comm_deinit(void);
HAL_StatusTypeDef bl_comm_disable_unused(void);

uint8_t bl_comm_select_port(bl_comm_port_t port);
bl_comm_port_t bl_comm_selected_port(void);

uint32_t bl_comm_read(uint8_t *data, uint32_t length);
uint8_t bl_comm_rx_overflow(void);
void bl_comm_rx_reset(void);

#endif