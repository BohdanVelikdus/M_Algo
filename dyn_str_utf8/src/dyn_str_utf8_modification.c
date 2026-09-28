#include "dyn_str_utf8.h"

#include <string.h>

static void reverse_bytes_range(unsigned char *start, unsigned char *end) {
    while (start < end) {
        const unsigned char tmp = *start;
        *start++ = *end;
        *end-- = tmp;
    }
}

bool dyn_str_utf8_is_utf8_whitespace(const uint32_t cp) {
    return cp == ' '  || cp == '\t' || cp == '\n' || cp == '\r' ||
           cp == '\v' || cp == '\f' ||
           cp == 0x00A0 || // Non-breaking space
           cp == 0x1680 || // Ogham space mark
           (cp >= 0x2000 && cp <= 0x200A) || // En space, Em space, Thin space, etc.
           cp == 0x2028 || // Line separator
           cp == 0x2029 || // Paragraph separator
           cp == 0x202F || // Narrow no-break space
           cp == 0x205F || // Medium mathematical space
           cp == 0x3000;   // Ideographic space (CJK)
}

/**
 * @brief Apennds a codepoint to a string
 * @param str dst string
 * @param cp codepoint to be appended
 * @return True - success
 * @return False - allocation error
 */
bool dyn_str_utf8_append_codepoint(dyn_str_utf8_t *str, const uint32_t cp)
{
    if (str == NULL) return false;

    const utf8_delim_bytes_t bytes = dyn_str_utf8_encode_utf8(cp);
    if (bytes.len == 0 || bytes.len > 4) {
        return false;
    }

    const size_t required_cap = str->size + bytes.len;
    if (str->capacity < required_cap) {
        if (!dyn_str_utf8_grow(str, required_cap)) {
            return false;
        }
    }

    memcpy(str->ptr + str->size, bytes.bytes, bytes.len);
    str->size += bytes.len;
    return true;
}

/**
 * @brief Remove last codepoint
 * @param str src string
 * @return True - success remove
 * @return False - invalid codepoint at the end
 */
bool dyn_str_utf8_pop_back_codepoint(dyn_str_utf8_t *str)
{
    if (str == NULL || str->ptr == NULL || str->size == 0) {
        return false;
    }

    size_t i = str->size;
    size_t bytes_to_remove = 0;

    while (i > 0) {
        i--;
        bytes_to_remove++;

        if ((str->ptr[i] & 0xC0) != 0x80) {
            break;
        }

        if (bytes_to_remove > 4) {
            return false;
        }
    }
    str->size -= bytes_to_remove;
    return true;
}

/**
 * @brief Found a codepoint under desired position
 * @param dest string to which would part be appended
 * @param start_cp position where insert src
 * @param src src, from which copy would happen
 * @return True - success
 * @return False - error with allocation
 */
bool dyn_str_utf8_insert(dyn_str_utf8_t *dest, const size_t start_cp, const dyn_str_utf8_t *src)
{
    if (dest == NULL || src == NULL || src->ptr == NULL) return false;

    size_t byte_offset = 0;
    if (!dyn_str_utf8_codepoint_to_byte_offset(dest, start_cp, &byte_offset)) {
        return false;
    }

    if (src->size == 0) return true;

    const size_t bytes_to_move = dest->size - byte_offset;
    const size_t required_capacity = dest->size + src->size;

    const unsigned char *src_bytes = src->ptr;
    unsigned char *temp_src = NULL;

    if (dest == src) {
        temp_src = (unsigned char *)dest->allocator.malloc_fn(src->size);
        if (temp_src == NULL) return false;

        memcpy(temp_src, src->ptr, src->size);
        src_bytes = temp_src;
    }

    if (dest->capacity < required_capacity) {
        if (!dyn_str_utf8_grow(dest, required_capacity)) {
            if (temp_src) {
                dest->allocator.free_fn(temp_src);
            }
            return false;
        }
    }

    if (bytes_to_move > 0) {
        memmove(dest->ptr + byte_offset + src->size, dest->ptr + byte_offset, bytes_to_move);
    }

    memcpy(dest->ptr + byte_offset, src_bytes, src->size);
    dest->size += src->size;
    if (temp_src) {
        dest->allocator.free_fn(temp_src);
    }

    return true;
}

/**
 * @brief Found a codepoint under desired position
 * @param dest string to which would part be appended
 * @param start_cp position where insert src
 * @param codepoint codepoint to be inserted
 * @return True - success
 * @return False - error with allocation, invalid UTF-8 string
 */
bool dyn_str_utf8_insert_codepoint_uint32_t(dyn_str_utf8_t *dest, const size_t start_cp, const uint32_t codepoint)
{
    if (dest == NULL) return false;

    const utf8_delim_bytes_t bytes = dyn_str_utf8_encode_utf8(codepoint);
    if (bytes.len == 0)
    {
        return false;
    }

    size_t length = 0;
    if (!dyn_str_utf8_length(dest, &length))
    {
        return false;
    }
    if (start_cp > length)
    {
        return false;
    }

    const size_t desired_capacity = dest->size + bytes.len;
    if (desired_capacity > dest->capacity)
    {
        if (!dyn_str_utf8_grow(dest, desired_capacity))
        {
            return false;
        }
    }

    size_t byte_offset;
    if (!dyn_str_utf8_codepoint_to_byte_offset(dest, start_cp, &byte_offset))
    {
        return false;
    }

    const size_t bytes_to_move = dest->size - byte_offset;
    if (bytes_to_move > 0) {
        memmove(dest->ptr + byte_offset + bytes.len, dest->ptr + byte_offset, bytes_to_move);
    }
    memcpy(dest->ptr + byte_offset, bytes.bytes, bytes.len);

    dest->size += bytes.len;
    return true;
}

/**
 * @brief Found a codepoint under desired position
 * @param dest string to which would part be appended
 * @param start_cp position where insert src
 * @param ptr pointer to const char*, which would be inserted
 * @return True - success
 * @return False - error with allocation, invalid UTF-8 string
 */
bool dyn_str_utf8_insert_codepoint_char_ptr(dyn_str_utf8_t *dest, const size_t start_cp, const char *ptr)
{
    if (dest == NULL || ptr == NULL) {
        return false;
    }

    const size_t insert_len = strlen(ptr);
    if (insert_len == 0) {
        return true;
    }

    if (!dyn_str_utf8_is_valid_utf8_cstring((const utf8_byte*)ptr, insert_len)) {
        return false;
    }

    size_t byte_offset = 0;
    if (!dyn_str_utf8_codepoint_to_byte_offset(dest, start_cp, &byte_offset)) {
        return false;
    }

    const size_t desired_capacity = dest->size + insert_len;
    if (desired_capacity > dest->capacity) {
        if (!dyn_str_utf8_grow(dest, desired_capacity)) {
            return false;
        }
    }

    const size_t bytes_to_move = dest->size - byte_offset;
    if (bytes_to_move > 0) {
        memmove(dest->ptr + byte_offset + insert_len, dest->ptr + byte_offset, bytes_to_move);
    }

    memcpy(dest->ptr + byte_offset, ptr, insert_len);

    dest->size += insert_len;
    return true;
}

bool dyn_str_utf8_erase(dyn_str_utf8_t *str, const size_t start_cp, const size_t count_cp) // NOLINT(readability-non-const-parameter)
{
    if (str == NULL || str->ptr == NULL) return false;
    if (count_cp == 0) return true;

    size_t byte_offset_start = 0;
    if (!dyn_str_utf8_codepoint_to_byte_offset(str, start_cp, &byte_offset_start)) {
        return false;
    }

    size_t byte_offset_end = 0;
    if (!dyn_str_utf8_codepoint_to_byte_offset(str, start_cp + count_cp, &byte_offset_end)) {
        return false;
    }

    const size_t bytes_to_erase = byte_offset_end - byte_offset_start;
    if (bytes_to_erase == 0) return true;

    const size_t tail_bytes = str->size - byte_offset_end;
    if (tail_bytes > 0) {
        memmove(str->ptr + byte_offset_start, str->ptr + byte_offset_end, tail_bytes);
    }

    str->size -= bytes_to_erase;
    str->ptr[str->size] = '\0';

    return true;
}

bool dyn_str_utf8_concat(dyn_str_utf8_t *dest, const dyn_str_utf8_t *src)
{
    if (dest == NULL || src == NULL ) return false;
    if (src->size == 0) return true;

    const size_t required = dest->size + src->size;
    if (dest->capacity < required) {
        size_t new_cap = dest->capacity == 0 ? required : dest->capacity * 2;
        if (new_cap < required) new_cap = required;
        if (!dyn_str_utf8_grow(dest, new_cap)) return false;
    }

    memcpy(dest->ptr + dest->size, src->ptr, src->size);
    dest->size = required;
    return true;
}

bool dyn_str_utf8_slice(const dyn_str_utf8_t *src, const size_t start_cp, const size_t count_cp, dyn_str_utf8_t *out_slice)
{
    if (src == NULL || out_slice == NULL || src->ptr == NULL) return false;

    size_t byte_start = 0;
    size_t current_cp = 0;

    while (byte_start < src->size && current_cp < start_cp) {
        size_t bytes = dyn_str_utf8_codepoint_bytes((uint8_t)src->ptr[byte_start]);

        if (bytes == 0 || (byte_start + bytes > src->size)) {
            bytes = 1;
        }

        byte_start += bytes;
        current_cp++;
    }

    if (current_cp < start_cp) return false;

    size_t byte_end = byte_start;
    size_t sliced_cps = 0;

    while (byte_end < src->size && sliced_cps < count_cp) {
        size_t bytes = dyn_str_utf8_codepoint_bytes((uint8_t)src->ptr[byte_end]);

        if (bytes == 0 || (byte_end + bytes > src->size)) {
            bytes = 1;
        }

        byte_end += bytes;
        sliced_cps++;
    }

    const size_t slice_bytes = byte_end - byte_start;
    if (src == out_slice) {
        if (byte_start > 0 && slice_bytes > 0) {
            memmove(out_slice->ptr, &src->ptr[byte_start], slice_bytes);
        }
        out_slice->size = slice_bytes;
        return true;
    }

    if (out_slice->capacity < slice_bytes || out_slice->ptr == NULL) {
        const size_t new_cap = (slice_bytes == 0) ? 1 : slice_bytes;
        if (!dyn_str_utf8_grow(out_slice, new_cap)) return false;
    }

    if (slice_bytes > 0) {
        memcpy(out_slice->ptr, &src->ptr[byte_start], slice_bytes);
    }
    out_slice->size = slice_bytes;

    return true;
}

bool dyn_str_utf8_reverse(dyn_str_utf8_t *str) // NOLINT(readability-non-const-parameter)
{
    if (str == NULL || str->ptr == NULL || str->size <= 1) return true;

    size_t i = 0;
    while (i < str->size) {
        size_t bytes = dyn_str_utf8_codepoint_bytes((uint8_t)str->ptr[i]);
        if (bytes == 0 || (i + bytes > str->size)) {
            bytes = 1;
        }
        if (bytes > 1) {
            reverse_bytes_range(&str->ptr[i], &str->ptr[i + bytes - 1]);
        }
        i += bytes;
    }
    reverse_bytes_range(&str->ptr[0], &str->ptr[str->size - 1]);
    return true;
}



bool dyn_str_utf8_trim(dyn_str_utf8_t *str) // NOLINT(readability-non-const-parameter)
{
    if (str == NULL || str->ptr == NULL) return false;
    if (str->size == 0) return true;

    size_t start_byte = 0;
    while (start_byte < str->size) {
        size_t cp_len = dyn_str_utf8_codepoint_bytes((uint8_t)str->ptr[start_byte]);

        if (cp_len == 0 || start_byte + cp_len > str->size) {
            cp_len = 1;
        }

        const uint32_t cp = utf8_bytes_to_uint32((const char*)&str->ptr[start_byte]);

        if (!dyn_str_utf8_is_utf8_whitespace(cp)) {
            break;
        }

        start_byte += cp_len;
    }

    if (start_byte >= str->size) {
        str->size = 0;
        return true;
    }

    size_t end_byte = str->size;
    while (end_byte > start_byte) {
        size_t scan = end_byte - 1;
        while (scan > start_byte && ((uint8_t)str->ptr[scan] & 0xC0) == 0x80) {
            scan--;
        }

        size_t cp_len = dyn_str_utf8_codepoint_bytes((uint8_t)str->ptr[scan]);
        if (cp_len == 0 || scan + cp_len > end_byte) {
            cp_len = 1;
        }

        const uint32_t cp = utf8_bytes_to_uint32((const char*)&str->ptr[scan]);

        if (!dyn_str_utf8_is_utf8_whitespace(cp)) {
            break;
        }

        end_byte = scan;
    }

    const size_t new_size = end_byte - start_byte;
    if (start_byte > 0 && new_size > 0) {
        memmove(str->ptr, str->ptr + start_byte, new_size);
    }
    str->size = new_size;
    return true;
}

bool dyn_str_utf8_replace(dyn_str_utf8_t *str, const dyn_str_utf8_t *target, const dyn_str_utf8_t *replacement)
{
    if (str == NULL || target == NULL || replacement == NULL) return false;
    if (target->size == 0) return false;

    const intptr_t match_byte_offset = dyn_str_utf8_find(str, target);
    if (match_byte_offset < 0) {
        return false;
    }

    const size_t offset = (size_t)match_byte_offset;

    if (replacement->size > target->size) {
        const size_t growth_needed = replacement->size - target->size;
        const size_t required_capacity = str->size + growth_needed;

        if (str->capacity < required_capacity) {
            if (!dyn_str_utf8_grow(str, required_capacity)) {
                return false;
            }
        }
        const size_t tail_bytes = str->size - (offset + target->size);
        if (tail_bytes > 0) {
            memmove(str->ptr + offset + replacement->size,
                    str->ptr + offset + target->size,
                    tail_bytes);
        }
        str->size += growth_needed;

    } else if (replacement->size < target->size) {
        const size_t shrink_amount = target->size - replacement->size;
        const size_t tail_bytes = str->size - (offset + target->size);

        if (tail_bytes > 0) {
            memmove(str->ptr + offset + replacement->size,
                    str->ptr + offset + target->size,
                    tail_bytes);
        }
        str->size -= shrink_amount;
    }

    if (replacement->size > 0) {
        memcpy(str->ptr + offset, replacement->ptr, replacement->size);
    }
    return true;
}
