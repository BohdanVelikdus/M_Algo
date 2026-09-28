#include "dyn_str_utf8.h"

#include <string.h>

/**
 * @brief Constructor for a utf8 string. Must be called before using struct object.
 * Uses STDLIB_ALLOCATOR for allocating memory(malloc, realloc, free)
 * @param str destination string
 * @param initial_capacity initial capacity of string
 * @return True - in case of success
 * @return False - in case of an error allocating memory
 */
bool dyn_str_utf8_init(dyn_str_utf8_t *str, const size_t initial_capacity)
{
    if (str == NULL || initial_capacity <= 0) return false;
    str->allocator = STDLIB_ALLOCATOR;

    str->ptr = str->allocator.malloc_fn(str->allocator.user_data, sizeof(char) * initial_capacity);
    if (str->ptr == NULL) return false;
    str->size = 0;
    str->capacity = initial_capacity;
    return true;
}

/**
 * @brief Constructor for a utf8 string. Must be called before using struct object. Accepts allocator
 * @param str destination string
 * @param initial_capacity initial capacity of string
 * @param allocator a structure, which holds a pointers to custom functions for allocating a memory
 * @return True - in case of success
 * @return False - in case of an error allocating memory
 */
bool dyn_str_utf8_init_with_allocator(dyn_str_utf8_t *str, const dyn_allocator_t allocator, const size_t initial_capacity)
{
    if (str == NULL || initial_capacity <= 0) return false;
    str->allocator = allocator;

    str->ptr = str->allocator.malloc_fn(str->allocator.user_data, sizeof(char) * initial_capacity);
    if (str->ptr == NULL) return false;
    str->size = 0;
    str->capacity = initial_capacity;
    return true;
}

/**
 * @brief Constructor for a utf8 string. OBJECT MUST BE ZEROED BEFORE USING THIS FUNCTION. Use c string for copying from
 * @param str destination string
 * @param allocator a structure, which holds a pointers to custom functions for allocating a memory
 * @param cstr a c-string, from which buffer would be constructed
 * @return True - in case of success
 * @return False - in case of an error allocating memory, or invalid UTF-8
 */
bool dyn_str_utf8_from_cstr(dyn_str_utf8_t *str, const dyn_allocator_t allocator, const char *cstr)
{
    if (str == NULL || cstr == NULL) return false;
    str->allocator = allocator;

    const size_t len = strlen(cstr);
    if (!dyn_str_utf8_is_valid_utf8_cstring((const utf8_byte*)cstr, len))
    {
        return false;
    }

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

/**
 * @brief Destructor for a utf8 string. Must be called after end of life of object
 * @param str object to be destroyed
 */
void dyn_str_utf8_destroy(dyn_str_utf8_t *str)
{
    if (str == NULL) return;

    if (str->ptr != NULL) {
        str->allocator.free_fn(str->allocator.user_data, str->ptr);
    }
    str->ptr = NULL;
    str->size = 0;
    str->capacity = 0;
    memset(&str->allocator, 0, sizeof(dyn_allocator_t));
}
