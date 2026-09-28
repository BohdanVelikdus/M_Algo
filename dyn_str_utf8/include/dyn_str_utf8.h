#ifndef DYN_STR_UTF8_H
#define DYN_STR_UTF8_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CHUNK_SIZE 512

typedef unsigned char utf8_byte;

typedef void* (*dyn_malloc_fn)(void *user_data, size_t size);
typedef void* (*dyn_realloc_fn)(void *user_data, void *ptr, size_t new_size);
typedef void  (*dyn_free_fn)(void *user_data, void *ptr);

typedef struct
{
    dyn_malloc_fn malloc_fn;
    dyn_realloc_fn realloc_fn;
    dyn_free_fn free_fn;
    void *user_data;
} dyn_allocator_t;

typedef struct
{
    utf8_byte *ptr;
    size_t size;
    size_t capacity;
    dyn_allocator_t allocator;
} dyn_str_utf8_t;

typedef struct {
    utf8_byte bytes[4];
    size_t len;
} utf8_delim_bytes_t;

void* wrapped_malloc(void *user_data, size_t size);

void* wrapped_realloc(void *user_data, void *ptr, size_t size);

void wrapped_free(void *user_data, void *ptr);

// Allocator
static const dyn_allocator_t STDLIB_ALLOCATOR = { .malloc_fn = wrapped_malloc, .realloc_fn = wrapped_realloc, .free_fn = wrapped_free };



#define DYN_STR_UTF8_ZERO { \
    .ptr = NULL, \
    .size = 0, \
    .capacity = 0, \
    .allocator = STDLIB_ALLOCATOR \
}

// Lifetime
bool dyn_str_utf8_init(dyn_str_utf8_t *str, size_t initial_capacity);
bool dyn_str_utf8_init_with_allocator(dyn_str_utf8_t *str, dyn_allocator_t allocator, size_t initial_capacity);
bool dyn_str_utf8_from_cstr(dyn_str_utf8_t *str, dyn_allocator_t allocator, const char *cstr);
void dyn_str_utf8_destroy(dyn_str_utf8_t *str);

// Copy & Swap
bool dyn_str_utf8_copy(const dyn_str_utf8_t *src, dyn_str_utf8_t *dst);
bool dyn_str_utf8_move(dyn_str_utf8_t *src, dyn_str_utf8_t *dst);

// Memory
void dyn_str_utf8_clear(dyn_str_utf8_t *str);
bool dyn_str_utf8_reserve(dyn_str_utf8_t *str, size_t new_capacity);
bool dyn_str_utf8_grow(dyn_str_utf8_t *str, size_t new_capacity);
bool dyn_str_utf8_shrink_to_fit(dyn_str_utf8_t *str);

// Validation
bool dyn_str_utf8_is_valid(const dyn_str_utf8_t *str);
bool dyn_str_utf8_is_valid_utf8_cstring(const utf8_byte *cstring, size_t str_len);
bool dyn_str_utf8_is_empty(const dyn_str_utf8_t *str);

// Inspection & Search
bool dyn_str_utf8_codepoint_to_byte_offset(const dyn_str_utf8_t *src, size_t target_cp, size_t *out_byte_offset);
bool dyn_str_utf8_length(const dyn_str_utf8_t *str, size_t *out_len);
bool dyn_str_utf8_equals(const dyn_str_utf8_t *lhs, const dyn_str_utf8_t *rhs);
intptr_t dyn_str_utf8_find(const dyn_str_utf8_t *haystack, const dyn_str_utf8_t *needle);
bool dyn_str_utf8_starts_with(const dyn_str_utf8_t *str, const dyn_str_utf8_t *prefix);
bool dyn_str_utf8_ends_with(const dyn_str_utf8_t *str, const dyn_str_utf8_t *suffix);
bool dyn_str_utf8_at(const dyn_str_utf8_t *str, size_t index, utf8_byte *out_byte);
bool dyn_str_utf8_at_codepoint(const dyn_str_utf8_t *str, size_t cp_index, uint32_t *out_codepoint);

// Modification
bool dyn_str_utf8_append_codepoint(dyn_str_utf8_t *str, uint32_t cp);
bool dyn_str_utf8_pop_back_codepoint(dyn_str_utf8_t *str);
bool dyn_str_utf8_insert(dyn_str_utf8_t *dest, size_t start_cp, const dyn_str_utf8_t *src);
bool dyn_str_utf8_insert_codepoint_uint32_t(dyn_str_utf8_t *dest, size_t start_cp, uint32_t codepoint);
bool dyn_str_utf8_insert_codepoint_char_ptr(dyn_str_utf8_t *dest, size_t start_cp, const char* ptr);
bool dyn_str_utf8_erase(dyn_str_utf8_t *str, size_t start_cp, size_t count_cp);
bool dyn_str_utf8_concat(dyn_str_utf8_t *dest, const dyn_str_utf8_t *src);
bool dyn_str_utf8_slice(const dyn_str_utf8_t *src, size_t start_cp, size_t count_cp, dyn_str_utf8_t *out_slice);
bool dyn_str_utf8_reverse(dyn_str_utf8_t *str);
bool dyn_str_utf8_is_utf8_whitespace(uint32_t cp);
bool dyn_str_utf8_trim(dyn_str_utf8_t *str);
bool dyn_str_utf8_replace(dyn_str_utf8_t *str, const dyn_str_utf8_t *target, const dyn_str_utf8_t *replacement);

// I/O & Streaming
void dyn_str_utf8_print_console(const dyn_str_utf8_t *str);
void dyn_str_utf8_print_stream(const dyn_str_utf8_t *str, FILE *stream);
bool dyn_str_utf8_read_line_console_chunked(dyn_str_utf8_t *str, uint32_t delimiter);
bool dyn_str_utf8_read_line_stream_chunked(dyn_str_utf8_t *str, uint32_t delimiter, FILE *stream);

// Encoding Utilities
size_t dyn_str_utf8_codepoint_bytes(utf8_byte byte);
utf8_delim_bytes_t dyn_str_utf8_encode_utf8(uint32_t cp);
uint32_t utf8_bytes_to_uint32(const char *utf8_str);


#ifdef __cplusplus
}
#endif

#endif