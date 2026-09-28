#include "dyn_str_utf8.h"

void* wrapped_malloc(void *user_data, const size_t size) // NOLINT(readability-non-const-parameter)
{
    (void)user_data;
    return malloc(size);
}

void* wrapped_realloc(void *user_data, void *ptr, const size_t size) // NOLINT(readability-non-const-parameter)
{
    (void)user_data;
    return realloc(ptr, size);
}

void wrapped_free(void *user_data, void *ptr) // NOLINT(readability-non-const-parameter)
{
    (void)user_data;
    free(ptr);
}