#include <string.h>

#include "dyn_str_utf8.h"

/**
 * @brief Copy function
 * @param src source string
 * @param dst destination string
 * @return True - in case of success
 * @return False - in case of an error allocating memory
 */
bool dyn_str_utf8_copy(const dyn_str_utf8_t *src, dyn_str_utf8_t *dst)
{
    if (src == NULL || dst == NULL) return false;
    if (src == dst) return true;

    if (src->size > dst->capacity)
    {
        if (!dyn_str_utf8_grow(dst, src->size))
        {
            return false;
        }
    }

    if (src->size > 0 && src->ptr != NULL)
    {
        memcpy(dst->ptr, src->ptr, src->size);
    }

    dst->size = src->size;
    return true;
}

/**
 * @brief Consumes the src string, and fill dst.
 * @param src - source string
 * @param dst - destination string
 * @return True - in case of success
 * @return False - in case of an error allocating memory
 */
bool dyn_str_utf8_move(dyn_str_utf8_t *src, dyn_str_utf8_t *dst)
{
    if (src == NULL || dst == NULL) return false;
    if (src == dst) return true;

    dyn_str_utf8_destroy(dst);

    dst->allocator = src->allocator;
    dst->size      = src->size;
    dst->capacity  = src->capacity;
    dst->ptr       = src->ptr;

    memset(src, 0, sizeof(dyn_str_utf8_t));
    return true;
}