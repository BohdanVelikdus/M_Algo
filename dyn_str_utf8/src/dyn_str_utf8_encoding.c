#include "dyn_str_utf8.h"

utf8_delim_bytes_t dyn_str_utf8_encode_utf8(const uint32_t cp) {
    utf8_delim_bytes_t d = {0};
    if (cp <= 0x7F) {
        d.bytes[0] = (uint8_t)cp;
        d.len = 1;
    }
    else if (cp <= 0x7FF) {
        d.bytes[0] = (uint8_t)(0xC0 | (cp >> 6));
        d.bytes[1] = (uint8_t)(0x80 | (cp & 0x3F));
        d.len = 2;
    }
    else if (cp <= 0xFFFF) {
        if (cp >= 0xD800 && cp <= 0xDFFF) {
            return d;
        }
        d.bytes[0] = (uint8_t)(0xE0 | (cp >> 12));
        d.bytes[1] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        d.bytes[2] = (uint8_t)(0x80 | (cp & 0x3F));
        d.len = 3;
    }
    else if (cp <= 0x10FFFF) {
        d.bytes[0] = (uint8_t)(0xF0 | (cp >> 18));
        d.bytes[1] = (uint8_t)(0x80 | ((cp >> 12) & 0x3F));
        d.bytes[2] = (uint8_t)(0x80 | ((cp >> 6) & 0x3F));
        d.bytes[3] = (uint8_t)(0x80 | (cp & 0x3F));
        d.len = 4;
    }
    return d;
}

uint32_t utf8_bytes_to_uint32(const char *utf8_str) {
    const uint8_t *bytes = (const uint8_t *)utf8_str;
    if (bytes == NULL)
        return 0;

    if (bytes[0] <= 0x7F) {
        return bytes[0];
    }
    if ((bytes[0] & 0xE0) == 0xC0) {
        return ((bytes[0] & 0x1F) << 6) |
               (bytes[1] & 0x3F);
    }
    if ((bytes[0] & 0xF0) == 0xE0) {
        return ((bytes[0] & 0x0F) << 12) |
               ((bytes[1] & 0x3F) << 6)  |
               (bytes[2] & 0x3F);
    }
    if ((bytes[0] & 0xF8) == 0xF0) {
        return ((bytes[0] & 0x07) << 18) |
               ((bytes[1] & 0x3F) << 12) |
               ((bytes[2] & 0x3F) << 6)  |
               (bytes[3] & 0x3F);
    }

    return 0;
}

// size_t dyn_str_utf8_codepoint_bytes(const uint8_t byte) {
//     if ((byte & 0x80) == 0x00) return 1;
//     if ((byte & 0xE0) == 0xC0) return 2;
//     if ((byte & 0xF0) == 0xE0) return 3;
//     if ((byte & 0xF8) == 0xF0) return 4;
//     return 0;
// }

size_t dyn_str_utf8_codepoint_bytes(const uint8_t byte)
{
    // 1-byte ASCII (0x00..0x7F)
    if (byte <= 0x7F) {
        return 1;
    }

    // Continuation bytes (0x80..0xBF) and overlong 2-byte leads (0xC0..0xC1)
    if (byte < 0xC2) {
        return 0;
    }

    // 2-byte sequences (0xC2..0xDF)
    if (byte <= 0xDF) {
        return 2;
    }

    // 3-byte sequences (0xE0..0xEF)
    if (byte <= 0xEF) {
        return 3;
    }

    // 4-byte sequences (0xF0..0xF4) -> Cap strictly at 0xF4 to disallow > U+10FFFF
    if (byte <= 0xF4) {
        return 4;
    }

    // Reject 0xF5..0xFF
    return 0;
}