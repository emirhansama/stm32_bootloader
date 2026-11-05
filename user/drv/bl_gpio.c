#include "bl_gpio.h"
#include "bl_config.h"
#include "main.h"

#if BL_BOOT_PORT == BL_PORT_A
#define BL_BOOT_GPIO GPIOA
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()
#elif BL_BOOT_PORT == BL_PORT_B
#define BL_BOOT_GPIO GPIOB
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOB_CLK_ENABLE()
#elif BL_BOOT_PORT == BL_PORT_C
#define BL_BOOT_GPIO GPIOC
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOC_CLK_ENABLE()
#elif BL_BOOT_PORT == BL_PORT_D
#define BL_BOOT_GPIO GPIOD
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOD_CLK_ENABLE()
#elif BL_BOOT_PORT == BL_PORT_E
#define BL_BOOT_GPIO GPIOE
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOE_CLK_ENABLE()
#elif BL_BOOT_PORT == BL_PORT_F
#define BL_BOOT_GPIO GPIOF
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOF_CLK_ENABLE()
#elif BL_BOOT_PORT == BL_PORT_G
#define BL_BOOT_GPIO GPIOG
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOG_CLK_ENABLE()
#elif BL_BOOT_PORT == BL_PORT_H
#define BL_BOOT_GPIO GPIOH
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOH_CLK_ENABLE()
#elif BL_BOOT_PORT == BL_PORT_I
#define BL_BOOT_GPIO GPIOI
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOI_CLK_ENABLE()
#elif BL_BOOT_PORT == BL_PORT_J
#define BL_BOOT_GPIO GPIOJ
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOJ_CLK_ENABLE()
#elif BL_BOOT_PORT == BL_PORT_K
#define BL_BOOT_GPIO GPIOK
#define BL_BOOT_GPIO_CLK_ENABLE() __HAL_RCC_GPIOK_CLK_ENABLE()
#else
#error "Unsupported BL_BOOT_PORT"
#endif

#define BL_BOOT_GPIO_PIN ((uint16_t)(1 << BL_BOOT_PIN))

/**
 * @brief Configures the boot selection pin as a GPIO input.
 */
void bl_gpio_init(void)
{
    BL_BOOT_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = BL_BOOT_GPIO_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = BL_BOOT_PULL;
    HAL_GPIO_Init(BL_BOOT_GPIO, &gpio);
}

/**
 * @brief Returns 1 when the configured boot button is pressed.
 */
uint8_t bl_gpio_is_boot_pressed(void)
{
    return HAL_GPIO_ReadPin(BL_BOOT_GPIO, BL_BOOT_GPIO_PIN) ==
           (BL_BOOT_ACTIVE_LEVEL ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
