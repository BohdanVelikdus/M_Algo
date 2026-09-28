#include "dyn_str_utf8.h"

#include <string.h>

/**
 * @brief Returns a byte offset based on the
 * @param src string to be validated
 * @param target_cp target code point to be found
 * @param out_byte_offset out byte offset - return
 * @return True - in case of found valid byte offset
 * @return False - in case of invalid arguments
 */
bool dyn_str_utf8_codepoint_to_byte_offset(const dyn_str_utf8_t *src, const size_t target_cp, size_t *out_byte_offset)
{
    if (src == NULL || out_byte_offset == NULL) return false;
    if (src->ptr == NULL) return (target_cp == 0 && src->size == 0);

    size_t byte_offset = 0;
    size_t current_cp = 0;

    while (byte_offset < src->size && current_cp < target_cp) {
        const size_t bytes = dyn_str_utf8_codepoint_bytes(src->ptr[byte_offset]);

        if (bytes == 0 || (byte_offset + bytes > src->size)) {
            return false;
        }

        byte_offset += bytes;
        current_cp++;
    }

    if (current_cp < target_cp) {
        return false;
    }

    *out_byte_offset = byte_offset;
    return true;
}

/**
 * @brief Returns a length of string in a codepoints
 * @param str string to be validated
 * @param out_len out - value of length in codepoints
 * @return True - in case of valid UTF-8 string
 * @return False - in case of invalid UTF-8 string
 */
bool dyn_str_utf8_length(const dyn_str_utf8_t *str, size_t *out_len)
{
    if (out_len == NULL) return false;
    *out_len = 0;

    if (str == NULL) return false;
    if (str->ptr == NULL) return str->size == 0;

    const utf8_byte *bytes = str->ptr;
    size_t i = 0;
    size_t count = 0;

    while (i < str->size) {
        const size_t len = dyn_str_utf8_codepoint_bytes(bytes[i]);

        if (len == 0 || (i + len > str->size)) {
            return false;
        }

        for (size_t j = 1; j < len; ++j) {
            if ((bytes[i + j] & 0xC0) != 0x80) {
                return false;
            }
        }

        switch (bytes[i]) {
        case 0xE0:
            if (bytes[i + 1] < 0xA0) return false;
            break;
        case 0xED:
            if (bytes[i + 1] >= 0xA0) return false;
            break;
        case 0xF0:
            if (bytes[i + 1] < 0x90) return false;
            break;
        case 0xF4:
            if (bytes[i + 1] > 0x8F) return false;
            break;
        default:
            break;
        }

        i += len;
        count++;
    }

    *out_len = count;
    return true;
}

/**
 * @brief this.Equals(other)
 * @param lhs string arg
 * @param rhs string arg
 */
bool dyn_str_utf8_equals(const dyn_str_utf8_t *lhs, const dyn_str_utf8_t *rhs)
{
    if (lhs == NULL || rhs == NULL) return false;
    if (lhs->size != rhs->size) return false;
    if (lhs->size == 0) return true;

    return memcmp(lhs->ptr, rhs->ptr, lhs->size) == 0;
}

/**
 * @brief Try to found a substring in string. O( n * m)
 * @param haystack string to be searched
 * @param needle substring to found
 * @return True - in case of valid UTF-8 string
 * @return False - in case of invalid UTF-8 string
 */
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

/**
 * @brief Checks for prefix
 * @param str string to be searched
 * @param prefix prefix
 * @return True - is prefix
 * @return False - not prefix
 */
bool dyn_str_utf8_starts_with(const dyn_str_utf8_t *str, const dyn_str_utf8_t *prefix)
{
    if (str == NULL || prefix == NULL) return false;
    if (prefix->size == 0) return true;

    if (str->size < prefix->size) return false;
    if (str->ptr == NULL || prefix->ptr == NULL) return false;

    return memcmp(str->ptr, prefix->ptr, prefix->size) == 0;
}

/**
 * @brief Checks for suffix
 * @param str string to be searched
 * @param suffix prefix
 * @return True - is suffix
 * @return False - not suffix
 */
bool dyn_str_utf8_ends_with(const dyn_str_utf8_t *str, const dyn_str_utf8_t *suffix)
{
    if (str == NULL || suffix == NULL) return false;
    if (str->ptr == NULL || suffix->ptr == NULL) return false;
    if (suffix->size == 0) return true;
    if (str->size < suffix->size) return false;

    return memcmp(str->ptr + (str->size - suffix->size), suffix->ptr, suffix->size) == 0;
}

/**
 * @brief at function - returns a value of the byte under some index
 * @param str src string
 * @param index bytes offset
 * @param out_byte out - returned value
 * @return True - valid index
 * @return False - invalid index
 */
bool dyn_str_utf8_at(const dyn_str_utf8_t *str, const size_t index, utf8_byte *out_byte)
{
    if (str == NULL || index >= str->size || out_byte == NULL) return false;
    *out_byte = str->ptr[index];
    return true;
}

/**
 * @brief Found a codepoint under desired position
 * @param str src string
 * @param cp_index codepoint position to find
 * @param out_codepoint out - returned codepoint value
 * @return True - valid index
 * @return False - invalid index
 */
bool dyn_str_utf8_at_codepoint(const dyn_str_utf8_t *str, const size_t cp_index, uint32_t *out_codepoint)
{
    if (str == NULL || str->ptr == NULL || out_codepoint == NULL) return false;

    size_t byte_offset = 0;
    if (!dyn_str_utf8_codepoint_to_byte_offset(str, cp_index, &byte_offset)) {
        return false;
    }

    if (byte_offset >= str->size) {
        return false;
    }

    const uint32_t cp = utf8_bytes_to_uint32((const char *)&str->ptr[byte_offset]);

    if (cp == 0 && str->ptr[byte_offset] != 0) {
        return false;
    }

    *out_codepoint = cp;
    return true;
}
