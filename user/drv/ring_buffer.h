
#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes -------------------------------------------------------------------
#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/** @addtogroup Driver
  * @{
  */

/** @addtogroup ring_buffer
  * @{
  */

// Exported defines -----------------------------------------------------------
/** @defgroup ring_buffer_Exported_Defines Exported Defines
  * @{
  */

#define RING_BUFFER_CHANNEL_NUM		(8)

/**
  * @}
  */

// Exported macros ------------------------------------------------------------
/** @defgroup ring_buffer_Exported_Macros Exported Macros
  * @{
  */

#define RING_BUF_DECLARE(name, size8) \
    static uint8_t _ring_buffer_data_##name[size8]; \
    struct ring_buf name = { \
        .buffer = _ring_buffer_data_##name, \
        .size = size8, \
        .head = 0, \
        .tail = 0 \
    }

/**
  * @}
  */

// Exported types -------------------------------------------------------------
/** @defgroup ring_buffer_Exported_Types Exported Types
  * @{
  */

struct ring_buf {
    uint8_t *buffer;
    uint32_t size;
    volatile uint32_t head;
    volatile uint32_t tail;
};

/**
  * @}
  */

// Exported variables ---------------------------------------------------------
/** @addtogroup ring_buffer_Exported_Variables Exported Variables
  * @{
  */



/**
  * @}
  */

// Exported functions ---------------------------------------------------------
/** @addtogroup ring_buffer_Exported_Functions Exported Functions
  * @{
  */

/**
 * @brief Resets the circular buffer read and write positions.
 * @param buf Circular buffer descriptor.
 */
void ring_buf_reset(struct ring_buf *buf);
/**
 * @brief Checks whether the circular buffer contains any data.
 * @param buf Circular buffer descriptor.
 * @return True if empty, otherwise false.
 */
bool ring_buf_is_empty(struct ring_buf *buf);
/**
 * @brief Checks whether the circular buffer has any free space.
 * @param buf Circular buffer descriptor.
 * @return True if full, otherwise false.
 */
bool ring_buf_is_full(struct ring_buf *buf);
/**
 * @brief Returns the number of unread bytes in the circular buffer.
 * @param buf Circular buffer descriptor.
 * @return Number of queued bytes.
 */
uint32_t ring_buf_size_get(struct ring_buf *buf);
/**
 * @brief Returns the number of bytes available for writing.
 * @param buf Circular buffer descriptor.
 * @return Number of writable bytes.
 */
uint32_t ring_buf_space_get(struct ring_buf *buf);
/**
 * @brief Writes up to the available capacity into the circular buffer.
 * @param buf Circular buffer descriptor.
 * @param data Source bytes to enqueue.
 * @param size Maximum number of bytes to enqueue.
 * @return Number of bytes written.
 */
uint32_t ring_buf_put(struct ring_buf *buf, const uint8_t *data, uint32_t size);
/**
 * @brief Reads and removes up to the requested number of bytes.
 * @param buf Circular buffer descriptor.
 * @param data Destination for removed bytes; NULL discards them.
 * @param size Maximum number of bytes to remove.
 * @return Number of bytes read or discarded.
 */
uint32_t ring_buf_get(struct ring_buf *buf, uint8_t *data, uint32_t size);
/**
 * @brief Reads bytes without removing them from the circular buffer.
 * @param buf Circular buffer descriptor.
 * @param data Destination for bytes to inspect.
 * @param size Maximum number of bytes to inspect.
 * @return Number of bytes copied.
 */
uint32_t ring_buf_peek(struct ring_buf *buf, uint8_t *data, uint32_t size);
/**
 * @brief Initializes a circular buffer with caller-provided storage.
 * @param buf Circular buffer descriptor.
 * @param size Storage capacity in bytes (minimum two).
 * @param data Caller-allocated backing storage.
 */
void ring_buf_init(struct ring_buf *buf, uint32_t size, uint8_t *data);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif // __RING_BUFFER_H__


