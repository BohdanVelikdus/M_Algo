#include "dyn_str_utf8.h"

#include <string.h>

void dyn_str_utf8_print_console(const dyn_str_utf8_t *str)
{
    dyn_str_utf8_print_stream(str, stdout);
}

void dyn_str_utf8_print_stream(const dyn_str_utf8_t *str, FILE *stream)
{
    if (str == NULL || str->ptr == NULL || str->size == 0) {
        fprintf(stream,"[empty]\n");
        return;
    }
    fprintf(stream, "%.*s\n", (int)str->size, str->ptr);
}


bool dyn_str_utf8_read_line_console_chunked(dyn_str_utf8_t *str, const uint32_t delimiter)
{
    return dyn_str_utf8_read_line_stream_chunked(str, delimiter, stdin);
}

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
            const size_t new_cap = (str->capacity == 0) ? CHUNK_SIZE : str->capacity * 2;
            if (!dyn_str_utf8_grow(str, new_cap)) {
                return false;
            }
        }

        str->ptr[str->size++] = (char)ch;

        if ((uint8_t)ch == delim.bytes[match_idx]) {
            match_idx++;
            if (match_idx == delim.len) {
                str->size -= delim.len;
                memset(str->ptr + str->size, 0, delim.len);
                return true;
            }
        } else {
            match_idx = ((uint8_t)ch == delim.bytes[0]) ? 1 : 0;
        }
    }

    return str->size > 0;
}