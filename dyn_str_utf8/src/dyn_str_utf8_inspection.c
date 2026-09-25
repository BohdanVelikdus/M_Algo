#include "dyn_str_utf8.h"

#include <string.h>

bool dyn_str_utf8_length(const dyn_str_utf8_t *str, size_t *out_len)
{
    if (out_len == NULL) {
        return false;
    }
    *out_len = 0;

    if (str == NULL || str->ptr == NULL) {
        return str && str->size == 0;
    }

    size_t count = 0;
    for (size_t i = 0; i < str->size; )
    {
        const size_t bytes = dyn_str_utf8_codepoint_bytes((uint8_t)str->ptr[i]);
        if (bytes == 0 || (i + bytes > str->size)) {
            return false;
        }

        i += bytes;
        count++;
    }

    *out_len = count;
    return true;
}

bool dyn_str_utf8_equals(const dyn_str_utf8_t *a, const dyn_str_utf8_t *b)
{
    if (a == NULL || b == NULL) return false;
    if (a->size != b->size) return false;
    if (a->size == 0) return true;

    return memcmp(a->ptr, b->ptr, a->size) == 0;
}

intptr_t dyn_str_utf8_find(const dyn_str_utf8_t *haystack, const dyn_str_utf8_t *needle)
{
    if (haystack == NULL || needle == NULL || needle->size == 0 || needle->size > haystack->size) {
        return -1;
    }

    for (size_t i = 0; i <= haystack->size - needle->size; ++i) {
        if (memcmp(haystack->ptr + i, needle->ptr, needle->size) == 0) {
            return (intptr_t)i;
        }
    }

    return -1;
}

bool dyn_str_utf8_starts_with(const dyn_str_utf8_t *str, const dyn_str_utf8_t *prefix)
{
    if (str == NULL || prefix == NULL) return false;
    if (prefix->size == 0) return true;

    if (str->size < prefix->size) return false;
    if (str->ptr == NULL || prefix->ptr == NULL) return false;

    return memcmp(str->ptr, prefix->ptr, prefix->size) == 0;
}

bool dyn_str_utf8_ends_with(const dyn_str_utf8_t *str, const dyn_str_utf8_t *suffix)
{
    if (str == NULL || suffix == NULL) return false;
    if (str->ptr == NULL || suffix->ptr == NULL) return false;
    if (suffix->size == 0) return true;
    if (str->size < suffix->size) return false;

    return memcmp(str->ptr + (str->size - suffix->size), suffix->ptr, suffix->size) == 0;
}
