#include "dyn_str_utf8.h"

/**
 * @brief Size = 0. Does not impact capacity. Zero the first char, if the capacity > 0
 * @param str destination string
 */
void dyn_str_utf8_clear(dyn_str_utf8_t *str)
{
    if (str == NULL) return;
    if (str->capacity != 0)
    {
        str->ptr[0] = '\0';
    }
    str->size = 0;
}

/**
 * @brief Allocates more memory. Does not impacts size
 * @param str destination string
 * @param new_capacity new capacity
 * @return True - in case of success
 * @return False - in case of an error allocating memory
 */
bool dyn_str_utf8_reserve(dyn_str_utf8_t *str, const size_t new_capacity)
{
    return dyn_str_utf8_grow(str, new_capacity);
}

/**
 * @brief Internal implementation of the allocating function
 * @param str destination string
 * @param new_capacity new capacity
 * @return True - in case of success
 * @return False - in case of an error allocating memory
 */
bool dyn_str_utf8_grow(dyn_str_utf8_t *str, const size_t new_capacity)
{
    if (str == NULL || new_capacity == 0) return false;

    utf8_byte *new_ptr = str->allocator.realloc_fn(str->allocator.user_data, str->ptr, new_capacity);
    if (new_ptr == NULL) return false;

    str->ptr = new_ptr;
    str->capacity = new_capacity;
    return true;
}

/**
 * @brief Deallocate unused memory. Reallocate the ptr. Frees unused memory
 * @param str destination string
 * @return True - in case of success
 * @return False - in case of an error allocating memory
 */
bool dyn_str_utf8_shrink_to_fit(dyn_str_utf8_t *str)
{
    if (str == NULL || str->ptr == NULL) return false;

    if (str->capacity == str->size) return true;

    const size_t new_capacity = (str->size == 0) ? 1 : str->size;

    unsigned char *shunk_ptr = str->allocator.realloc_fn(str->allocator.user_data, str->ptr, new_capacity);
    if (shunk_ptr == NULL) return false;

    str->ptr = shunk_ptr;
    str->capacity = new_capacity;

    return true;
}