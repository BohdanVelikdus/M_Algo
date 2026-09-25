#include "dyn_str_utf8.h"

void dyn_str_utf8_clear(dyn_str_utf8_t *str)
{
    if (str == NULL) return;
    if (str->capacity != 0)
    {
        str->ptr[0] = '\0';
    }
    str->size = 0;
}

bool dyn_str_utf8_reserve(dyn_str_utf8_t *str, const size_t new_capacity)
{
    return dyn_str_utf8_grow(str, new_capacity);
}

bool dyn_str_utf8_grow(dyn_str_utf8_t *str, const size_t new_capacity)
{
    if (str == NULL || new_capacity == 0) return false;

    char *new_ptr = str->allocator.realloc_fn(str->ptr, new_capacity);
    if (new_ptr == NULL) return false;

    str->ptr = new_ptr;
    str->capacity = new_capacity;
    return true;
}

bool dyn_str_utf8_shrink_to_fit(dyn_str_utf8_t *str)
{
    if (str == NULL || str->ptr == NULL) return false;

    if (str->capacity == str->size) return true;

    const size_t new_capacity = (str->size == 0) ? 1 : str->size;

    char *shunk_ptr = str->allocator.realloc_fn(str->ptr, new_capacity);
    if (shunk_ptr == NULL) return false;

    str->ptr = shunk_ptr;
    str->capacity = new_capacity;

    return true;
}