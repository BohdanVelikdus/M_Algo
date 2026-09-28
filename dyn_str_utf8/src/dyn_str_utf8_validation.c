#include "dyn_str_utf8.h"

/**
 * @brief Checks if the string contains invalid Unicode chars
 * @param str string to be validated
 * @return True - in case of valid UTF-8 string
 * @return False - in case of invalid UTF-8 string
 */
bool dyn_str_utf8_is_valid(const dyn_str_utf8_t *str)
{
    if (str == NULL) return false;
    if (str->ptr == NULL) return str->size == 0;

    return dyn_str_utf8_is_valid_utf8_cstring(str->ptr, str->size);
}

/**
 * @brief Check if internal buffer of string is empty or not
 * @param str destination string
 * @return True - empty
 * @return False - non-empty
 */
bool dyn_str_utf8_is_empty(const dyn_str_utf8_t *str)
{
    if (str == NULL) return false;
    return str->size > 0 ? false : true;
}

/**
 * @brief Check if the const unsigned char* is valid UTF-8 sequence
 * @param cstring passed c_string
 * @param str_len length (without a null terminator) of cstring
 * @return True - valid
 * @return False - invalid
 */
bool dyn_str_utf8_is_valid_utf8_cstring(const utf8_byte *cstring, const size_t str_len)
{
    if (cstring == NULL) return false;

    const utf8_byte *bytes = cstring;
    size_t i = 0;

    while (i < str_len) {
        const size_t len = dyn_str_utf8_codepoint_bytes(bytes[i]);

        // 1. Check for invalid lead byte or truncated sequence
        if (len == 0 || (i + len > str_len)) {
            return false;
        }

        // 2. Validate multi-byte continuation pattern (10xxxxxx)
        for (size_t j = 1; j < len; ++j) {
            if ((bytes[i + j] & 0xC0) != 0x80) {
                return false;
            }
        }

        // 3. Check 2nd-byte restrictions (Overlongs, Surrogates, Out-of-bounds)
        switch (bytes[i]) {
        case 0xE0: // Overlong 3-byte
            if (bytes[i + 1] < 0xA0) return false;
            break;
        case 0xED: // UTF-16 surrogates
            if (bytes[i + 1] >= 0xA0) return false;
            break;
        case 0xF0: // Overlong 4-byte
            if (bytes[i + 1] < 0x90) return false;
            break;
        case 0xF4: // Above U+10FFFF
            if (bytes[i + 1] > 0x8F) return false;
            break;
        default:
            break;
        }

        i += len;
    }
    return true;
}
