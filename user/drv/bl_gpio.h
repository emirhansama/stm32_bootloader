#ifndef BL_GPIO_H
#define BL_GPIO_H

#include <stdint.h>

void bl_gpio_init(void);
uint8_t bl_gpio_is_boot_pressed(void);

#endif /* BL_GPIO_H */
