#include "dyn_str_utf8.h"

#include <string.h>

bool dyn_str_utf8_init(dyn_str_utf8_t *str, const size_t initial_capacity)
{
    if (str == NULL || initial_capacity <= 0) return false;
    str->allocator = STDLIB_ALLOCATOR;

    str->ptr = str->allocator.malloc_fn(sizeof(char) * initial_capacity);
    if (str->ptr == NULL) return false;
    str->size = 0;
    str->capacity = initial_capacity;
    return true;
}

bool dyn_str_utf8_init_with_allocator(dyn_str_utf8_t *str, dyn_allocator_t allocator, size_t initial_capacity)
{
    if (str == NULL || initial_capacity <= 0) return false;
    str->allocator = allocator;

    str->ptr = str->allocator.malloc_fn(sizeof(char) * initial_capacity);
    if (str->ptr == NULL) return false;
    str->size = 0;
    str->capacity = initial_capacity;
    return true;
}

bool dyn_str_utf8_from_cstr(dyn_str_utf8_t *str, const char *cstr)
{
    if (str == NULL || cstr == NULL) return false;

    const size_t len = strlen(cstr);

    if (str->capacity < len || str->ptr == NULL)
    {
        const size_t new_cap = len == 0 ? 1 : len;
        if (!dyn_str_utf8_grow(str, new_cap))
        {
            return false;
        }
    }

    if (len > 0) {
        memcpy(str->ptr, cstr, len);
    }

    str->size = len;
    return true;
}

void dyn_str_utf8_destroy(dyn_str_utf8_t *str)
{
    if (str == NULL) return;
    str->allocator.free_fn(str->ptr);
    str->ptr = NULL;
    str->size = 0;
    str->capacity = 0;
}
