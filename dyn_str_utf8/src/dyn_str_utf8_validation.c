#include "dyn_str_utf8.h"

bool dyn_str_utf8_is_valid(const dyn_str_utf8_t *str)
{
    if (str == NULL) return false;
    if (str->ptr == NULL) return str->size == 0;

    const uint8_t *bytes = (const uint8_t *)str->ptr;
    size_t i = 0;

    while (i < str->size) {
        const size_t len = dyn_str_utf8_codepoint_bytes(bytes[i]);

        // 1. Check for invalid lead byte or truncated sequence
        if (len == 0 || (i + len > str->size)) {
            return false;
        }

        // 2. Validate multi-byte continuation patterns (must be 10xxxxxx -> 0x80 to 0xBF)
        for (size_t j = 1; j < len; ++j) {
            if ((bytes[i + j] & 0xC0) != 0x80) {
                return false;
            }
        }

        // 3. Strict UTF-8 Validation (overlong encodings & surrogate halves)
        if (len == 2) {
            // Reject overlong 2-byte sequences (code points < U+0080)
            if (bytes[i] < 0xC2) return false;
        } else if (len == 3) {
            // Reject overlong 3-byte sequences or UTF-16 surrogates (U+D800 - U+DFFF)
            if (bytes[i] == 0xE0 && bytes[i + 1] < 0xA0) return false; // Overlong
            if (bytes[i] == 0xED && bytes[i + 1] >= 0xA0) return false; // Surrogate
        } else if (len == 4) {
            if (bytes[i] == 0xF0 && bytes[i + 1] < 0x90) return false; // Overlong
            if (bytes[i] == 0xF4 && bytes[i + 1] > 0x8F) return false; // Out of Unicode range
        }

        i += len;
    }

    return true;
}

bool dyn_str_utf8_is_empty(const dyn_str_utf8_t *str)
{
    if (str == NULL) return false;
    return str->size > 0 ? false : true;
}