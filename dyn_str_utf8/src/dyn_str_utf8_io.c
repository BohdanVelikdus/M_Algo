#include "dyn_str_utf8.h"

#include <string.h>

/**
 * @brief Writes into stdout
 * @param str string to be printed into streamm
 */
void dyn_str_utf8_print_console(const dyn_str_utf8_t *str)
{
    dyn_str_utf8_print_stream(str, stdout);
}

/**
 * @brief Writes into Stream
 * @param str string to be printed into streamm
 * @param stream destination buffer for writing
 */
void dyn_str_utf8_print_stream(const dyn_str_utf8_t *str, FILE *stream)
{
    if (stream == NULL) return;

    if (str == NULL || str->ptr == NULL || str->size == 0) {
        fprintf(stream, "[empty]\n");
        return;
    }

    fprintf(stream, "%.*s\n", (int)str->size, (const char*)str->ptr);
}

/**
 * @brief Reads from stdio
 * @param str destination string
 * @param delimiter read until delimiter
 * @return False error read from stream(not enough memory, stream is invalid)
 * @return True successful read until delimiter, or EOF, but read some data from stream
 */
bool dyn_str_utf8_read_line_console_chunked(dyn_str_utf8_t *str, const uint32_t delimiter)
{
    return dyn_str_utf8_read_line_stream_chunked(str, delimiter, stdin);
}

/**
 * @brief Reads from FILE* into a string.
 * @param str destination string
 * @param delimiter read until delimiter
 * @param stream source of data for reading
 * @return False error read from stream(not enough memory, stream is invalid)
 * @return True successful read until delimiter, or EOF, but read some data from stream
 */
bool dyn_str_utf8_read_line_stream_chunked(dyn_str_utf8_t *str, const uint32_t delimiter, FILE *stream)
{
    if (str == NULL || stream == NULL) return false;

    const utf8_delim_bytes_t delim = dyn_str_utf8_encode_utf8(delimiter);
    if (delim.len == 0) return false;

    str->size = 0;
    int ch;
    size_t match_idx = 0;

    while ((ch = fgetc(stream)) != EOF) {
        if (str->size >= str->capacity) {
            const size_t new_cap = (str->capacity == 0) ? 64 : str->capacity * 2;
            if (!dyn_str_utf8_grow(str, new_cap)) {
                str->size = 0;
                if (str->capacity > 0) str->ptr[0] = '\0';
                return false;
            }
        }

        const utf8_byte byte_read = (utf8_byte)ch;
        str->ptr[str->size++] = byte_read;

        if (byte_read == delim.bytes[match_idx]) {
            match_idx++;
            if (match_idx == delim.len) {
                // Strip delimiter
                str->size -= delim.len;
                if (str->capacity > str->size) {
                    str->ptr[str->size] = '\0';
                }

                if (!dyn_str_utf8_is_valid(str)) {
                    str->size = 0;
                    if (str->capacity > 0) str->ptr[0] = '\0';
                    return false;
                }
                return true;
            }
        } else {
            match_idx = (byte_read == delim.bytes[0]) ? 1 : 0;
        }
    }

    if (str->size > 0) {
        if (str->capacity > str->size) {
            str->ptr[str->size] = '\0';
        }

        if (!dyn_str_utf8_is_valid(str)) {
            str->size = 0;
            if (str->capacity > 0) str->ptr[0] = '\0';
            return false;
        }
        return true;
    }

    return false;
}