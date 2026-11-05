

// Includes -------------------------------------------------------------------
#include "ring_buffer.h"

/** @addtogroup Driver
  * @{
  */

/** @defgroup ring_buffer RING_BUFFER
  * @brief Generic RING_BUFFER driver
  * @{
  */

// Private defines ------------------------------------------------------------
/** @defgroup ring_buffer_Private_Defines Private Defines
  * @{
  */


/**
  * @}
  */

// Private macros -------------------------------------------------------------
/** @defgroup ring_buffer_Private_Macros Private Macros
  * @{
  */

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

/**
  * @}
  */

// Private types --------------------------------------------------------------
/** @defgroup ring_buffer_Private_Types Private Types
  * @{
  */



/**
  * @}
  */

// Private variables ----------------------------------------------------------
/** @defgroup ring_buffer_Private_Variables Private Variables
  * @{
  */


/**
  * @}
  */

// Private constants ----------------------------------------------------------
/** @defgroup ring_buffer_Private_Constants Private Constants
  * @{
  */

/**
  * @}
  */

// Exported variables ---------------------------------------------------------
/** @defgroup ring_buffer_Exported_Variables Exported Variables
  * @{
  */


/**
  * @}
  */

// Private functions ----------------------------------------------------------
/** @defgroup ring_buffer_Private_Functions Private Functions
  * @{
  */



/**
  * @}
  */

// Exported functions ---------------------------------------------------------
/** @defgroup ring_buffer_Exported_Functions Exported Functions
  * @{
  */

/**
 * @brief Resets the circular buffer read and write positions.
 * @param buf Circular buffer descriptor.
 */
void ring_buf_reset(struct ring_buf *buf)
{
    buf->head = 0;
    buf->tail = 0;
}

  /**
 * @brief Checks whether the circular buffer contains any data.
 * @param buf Circular buffer descriptor.
 * @return True if empty, otherwise false.
 */
bool ring_buf_is_empty(struct ring_buf *buf)
{
    return (buf->head == buf->tail);
}

/**
 * @brief Checks whether the circular buffer has any free space.
 * @param buf Circular buffer descriptor.
 * @return True if full, otherwise false.
 */
bool ring_buf_is_full(struct ring_buf *buf)
{
    return ((buf->head + 1) % buf->size) == buf->tail;
}

/**
 * @brief Returns the number of unread bytes in the circular buffer.
 * @param buf Circular buffer descriptor.
 * @return Number of queued bytes.
 */
uint32_t ring_buf_size_get(struct ring_buf *buf)
{
    if (buf->head >= buf->tail) {
        return buf->head - buf->tail;
    } else {
        return (buf->size - buf->tail) + buf->head;
    }
}

/**
 * @brief Returns the number of bytes available for writing.
 * @param buf Circular buffer descriptor.
 * @return Number of writable bytes.
 */
uint32_t ring_buf_space_get(struct ring_buf *buf)
{
    // Tam doluluk durumunu ayırt etmek için 1 byte boşluk bırakılır
    uint32_t data_len = ring_buf_size_get(buf);
    return (buf->size - 1) - data_len;
}

/**
 * @brief Writes up to the available capacity into the circular buffer.
 * @param buf Circular buffer descriptor.
 * @param data Source bytes to enqueue.
 * @param size Maximum number of bytes to enqueue.
 * @return Number of bytes written.
 */
uint32_t ring_buf_put(struct ring_buf *buf, const uint8_t *data, uint32_t size)
{
    uint32_t space = ring_buf_space_get(buf);
    uint32_t to_copy = MIN(size, space);
    
    if (to_copy == 0) {
        return 0;
    }

    uint32_t head_idx = buf->head;
    uint32_t first_chunk = MIN(to_copy, buf->size - head_idx);

    // İlk parça: Head'den buffer sonuna kadar
    memcpy(&buf->buffer[head_idx], data, first_chunk);

    // İkinci parça: Eğer wrap-around olduysa buffer başından devam et
    if (to_copy > first_chunk) {
        memcpy(buf->buffer, &data[first_chunk], to_copy - first_chunk);
    }

    buf->head = (head_idx + to_copy) % buf->size;

    return to_copy;
}

/**
 * @brief Reads and removes up to the requested number of bytes.
 * @param buf Circular buffer descriptor.
 * @param data Destination for removed bytes; NULL discards them.
 * @param size Maximum number of bytes to remove.
 * @return Number of bytes read or discarded.
 */
uint32_t ring_buf_get(struct ring_buf *buf, uint8_t *data, uint32_t size)
{
    uint32_t available = ring_buf_size_get(buf);
    uint32_t to_read = MIN(size, available);

    if (to_read == 0) {
        return 0;
    }

    uint32_t tail_idx = buf->tail;
    uint32_t first_chunk = MIN(to_read, buf->size - tail_idx);

    // İlk parça: Tail'den buffer sonuna kadar
    if (data != NULL) {
        memcpy(data, &buf->buffer[tail_idx], first_chunk);
    }

    // İkinci parça: Buffer başından
    if (to_read > first_chunk) {
        if (data != NULL) {
            memcpy(&data[first_chunk], buf->buffer, to_read - first_chunk);
        }
    }

    buf->tail = (tail_idx + to_read) % buf->size;

    return to_read;
}

/**
 * @brief Reads bytes without removing them from the circular buffer.
 * @param buf Circular buffer descriptor.
 * @param data Destination for bytes to inspect.
 * @param size Maximum number of bytes to inspect.
 * @return Number of bytes copied.
 */
uint32_t ring_buf_peek(struct ring_buf *buf, uint8_t *data, uint32_t size)
{
    uint32_t available = ring_buf_size_get(buf);
    uint32_t to_read = MIN(size, available);

    if (to_read == 0) {
        return 0;
    }

    uint32_t tail_idx = buf->tail;
    uint32_t first_chunk = MIN(to_read, buf->size - tail_idx);

    memcpy(data, &buf->buffer[tail_idx], first_chunk);

    if (to_read > first_chunk) {
        memcpy(&data[first_chunk], buf->buffer, to_read - first_chunk);
    }

    // Tail güncellenmez!
    return to_read;
}

/**
 * @brief Initializes a circular buffer with caller-provided storage.
 * @param buf Circular buffer descriptor.
 * @param size Storage capacity in bytes (minimum two).
 * @param data Caller-allocated backing storage.
 */
void ring_buf_init(struct ring_buf *buf, uint32_t size, uint8_t *data)
{
    buf->buffer = data;
    buf->size = size;
    buf->head = 0;
    buf->tail = 0;
}


