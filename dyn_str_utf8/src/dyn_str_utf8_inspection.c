#include "dyn_str_utf8.h"

#include <string.h>

bool dyn_str_utf8_length(const dyn_str_utf8_t *str, size_t *out_len)
{
    if (out_len == NULL) {
        return false;
    }
    *out_len = 0;

    if (!dyn_str_utf8_is_valid(str)) {
        return false;
    }

    if (str->size == 0) {
        return true;
    }

    const uint8_t *bytes = (const uint8_t *)str->ptr;
    size_t count = 0;

    for (size_t i = 0; i < str->size; ) {
        i += dyn_str_utf8_codepoint_bytes(bytes[i]);
        count++;
    }

    *out_len = count;
    return true;
}

bool dyn_str_utf8_equals(const dyn_str_utf8_t *lhs, const dyn_str_utf8_t *rhs)
{
    if (lhs == NULL || rhs == NULL) return false;
    if (lhs->size != rhs->size) return false;
    if (lhs->size == 0) return true;

    return memcmp(lhs->ptr, rhs->ptr, lhs->size) == 0;
}

intptr_t dyn_str_utf8_find(const dyn_str_utf8_t *haystack, const dyn_str_utf8_t *needle)
{
    if (haystack == NULL || needle == NULL || needle->size > haystack->size) {
        return -1;
    }

    if (needle->size == 0)
        return 0;

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
