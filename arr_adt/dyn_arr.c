#include "dyn_arr.h"

#include <stdlib.h>
#include <string.h>

static bool dyn_arr_grow(dyn_arr_t *arr) {
    const size_t new_capacity = (arr->capacity == 0) ? 2 : arr->capacity + (arr->capacity / 2) + 1;
    int *new_ptr = realloc(arr->ptr, sizeof(*arr->ptr) * new_capacity);
    if (!new_ptr) {
        return false;
    }
    arr->ptr = new_ptr;
    arr->capacity = new_capacity;
    return true;
}

bool dyn_arr_init(dyn_arr_t *arr, const size_t capacity) {
    if (arr == NULL || capacity == 0) return false;

    arr->ptr = malloc(sizeof(*arr->ptr) * capacity);
    if (!arr->ptr) return false;

    arr->capacity = capacity;
    arr->size = 0;
    return true;
}

void dyn_arr_destroy(dyn_arr_t *arr) {
    if (arr == NULL) return;
    free(arr->ptr);
    arr->ptr = NULL;
    arr->capacity = 0;
    arr->size = 0;
}

void dyn_arr_display(const dyn_arr_t *arr) {
    if (arr == NULL) return;
    printf("[ ");
    for (size_t i = 0; i < arr->size; ++i) {
        printf("%d ", arr->ptr[i]);
    }
    printf("]\n");
}

bool dyn_arr_append(dyn_arr_t *arr, int value) {
    if (arr == NULL) return false;

    if (arr->size >= arr->capacity) {
        if (!dyn_arr_grow(arr)) return false;
    }
    arr->ptr[arr->size++] = value;
    return true;
}


bool dyn_arr_insert(dyn_arr_t *arr, size_t index, int value) {
    if (arr == NULL || index > arr->size) return false;

    if (arr->size >= arr->capacity) {
        if (!dyn_arr_grow(arr)) return false;
    }

    if (index < arr->size) {
        memmove(&arr->ptr[index + 1], &arr->ptr[index], (arr->size - index) * sizeof(int));
    }

    arr->ptr[index] = value;
    arr->size++;
    return true;
}

bool dyn_arr_delete(dyn_arr_t *arr, const size_t index, int* out_value) {
    if (arr == NULL || index >= arr->size) return false;

    if (out_value != NULL) {
        *out_value = arr->ptr[index];
    }

    if (index < arr->size - 1) {
        memmove(&arr->ptr[index], &arr->ptr[index + 1], (arr->size - index - 1) * sizeof(int));
    }

    arr->size--;
    return true;
}

bool dyn_arr_search(const dyn_arr_t *arr, const int value)
{
    if (arr == NULL) return false;

    for (size_t i = 0; i < arr->size; ++i)
    {
        if (arr->ptr[i] == value) return true;
    }
    return false;
}

bool dyn_arr_get(const dyn_arr_t *arr, size_t index, int *out_value)
{
    if (arr == NULL || out_value == NULL || index >= arr->size) {
        return false;
    }
    *out_value = arr->ptr[index];
    return true;
}

bool dyn_arr_set(dyn_arr_t *arr, size_t index, int value)
{
    if (arr == NULL || index >= arr->size) return false;
    arr->ptr[index] = value;
    return true;
}

int max(const dyn_arr_t *arr)
{
    if (arr == NULL || arr->size == 0) return 0;

    int max = arr->ptr[0];
    for (size_t i = 1; i < arr->size; ++i)
    {
        if (arr->ptr[i] > max) max = arr->ptr[i];
    }
    return max;
}

int min(const dyn_arr_t *arr)
{
    if (arr == NULL || arr->size == 0) return 0;
    int min = arr->ptr[0];
    for (size_t i = 1; i < arr->size; ++i)
    {
        if (arr->ptr[i] < min) min = arr->ptr[i];
    }
    return min;
}

void dyn_arr_reverse(dyn_arr_t *arr)
{
    if (arr == NULL) return;

    for (size_t i = 0; i < arr->size/2; ++i)
    {
        const int tmp = arr->ptr[i];
        arr->ptr[i] = arr->ptr[arr->size - i - 1];
        arr->ptr[arr->size - i - 1] = tmp;
    }
}

int dyn_arr_sum(const dyn_arr_t *arr)
{
    if (arr == NULL || arr->size == 0) return 0;

    int sum = 0;
    for (size_t i = 0; i < arr->size; ++i)
    {
        sum += arr->ptr[i];
    }
    return sum;
}

double dyn_arr_avg(const dyn_arr_t *arr)
{
    if (arr == NULL || arr->size == 0) return 0;
    return (double)dyn_arr_sum(arr) / (double)arr->size;
}

bool dyn_arr_is_sorted(const dyn_arr_t *arr)
{
    if (arr == NULL) return false;
    if (arr->size <= 1) return true;

    for (size_t i = 0; i < arr->size - 1; ++i)
    {
        if (arr->ptr[i] > arr->ptr[i + 1]) {
            return false;
        }
    }
    return true;
}

bool dyn_arr_binary_search(const dyn_arr_t *arr, int value)
{
    if (arr == NULL || arr->size == 0) return false;

    if (!dyn_arr_is_sorted(arr)) return false;

    size_t low = 0;
    size_t high = arr->size - 1;

    while (low <= high) {
        size_t mid = low + (high - low) / 2;

        if (arr->ptr[mid] == value) {
            return (int)mid;
        }

        if (arr->ptr[mid] < value) {
            low = mid + 1;
        } else {
            if (mid == 0) break;
            high = mid - 1;
        }
    }

    return false;
}

static bool dyn_arr_realloc(dyn_arr_t *arr, size_t total_size)
{
    if (arr == NULL) return false;
    if (total_size == 0) return true;

    int *new_ptr = realloc(arr->ptr, sizeof(*arr->ptr) * total_size);
    if (!new_ptr) return false;
    arr->ptr = new_ptr;
    arr->capacity = total_size;
    return true;
}

bool dyn_arr_merge(const dyn_arr_t *rhs, const dyn_arr_t *lhs, dyn_arr_t *result)
{
    if (lhs == NULL || rhs == NULL || result == NULL) return false;

    size_t total_size = lhs->size + rhs->size;
    if (total_size == 0) return true;

    if (result->capacity < total_size) {
        if (!dyn_arr_realloc(result, total_size))
            return false;
    }

    if (lhs->size > 0) {
        memcpy(result->ptr, lhs->ptr, lhs->size * sizeof(int));
    }
    if (rhs->size > 0) {
        memcpy(result->ptr + lhs->size, rhs->ptr, rhs->size * sizeof(int));
    }

    result->size = total_size;
    return true;
}

bool dyn_arr_set_difference( const dyn_arr_t *lhs, const dyn_arr_t *rhs, dyn_arr_t *result)
{
    if (lhs == NULL || rhs == NULL || result == NULL) return false;

    if (result->capacity < lhs->size)
    {
        if (!dyn_arr_realloc(result, lhs->size))
            return false;
    }

    result->size = 0;

    for (size_t i = 0; i < lhs->size; ++i)
    {
        if (!dyn_arr_search(rhs, lhs->ptr[i]))
        {
            result->ptr[result->size++] = lhs->ptr[i];
        }
    }

    return true;
}

bool dyn_arr_set_intersection( const dyn_arr_t *rhs, const dyn_arr_t *lhs, dyn_arr_t *result)
{
    if (lhs == NULL || rhs == NULL || result == NULL) return false;

    if (result->capacity < lhs->size)
    {
        if (!dyn_arr_realloc(result, lhs->size))
            return false;
    }

    result->size = 0;

    for (size_t i = 0; i < lhs->size; ++i)
    {
        if (dyn_arr_search(rhs, lhs->ptr[i]) && !dyn_arr_search(result, lhs->ptr[i]))
        {
            result->ptr[result->size++] = lhs->ptr[i];
        }
    }

    return true;
}

bool dyn_arr_set_union(const dyn_arr_t *lhs, const dyn_arr_t *rhs, dyn_arr_t *result)
{
    if (lhs == NULL || rhs == NULL || result == NULL) return false;

    const size_t max_possible_size = lhs->size + rhs->size;
    if (result->capacity < max_possible_size)
    {
        if (!dyn_arr_realloc(result, max_possible_size))
            return false;
    }

    result->size = 0;

    for (size_t i = 0; i < lhs->size; ++i)
    {
        if (!dyn_arr_search(result, lhs->ptr[i]))
        {
            result->ptr[result->size++] = lhs->ptr[i];
        }
    }
    for (size_t i = 0; i < rhs->size; ++i)
    {
        if (!dyn_arr_search(result, rhs->ptr[i]))
        {
            result->ptr[result->size++] = rhs->ptr[i];
        }
    }
    return true;
}

void dyn_arr_shift_left(dyn_arr_t *arr) {
    if (arr == NULL || arr->size == 0) return;

    if (arr->size > 1) {
        memmove(&arr->ptr[0], &arr->ptr[1], (arr->size - 1) * sizeof(int));
    }
    arr->ptr[arr->size - 1] = 0;
}

void dyn_arr_shift_right(dyn_arr_t *arr) {
    if (arr == NULL || arr->size == 0) return;

    if (arr->size > 1) {
        memmove(&arr->ptr[1], &arr->ptr[0], (arr->size - 1) * sizeof(int));
    }
    arr->ptr[0] = 0;
}

void dyn_arr_rotate_left(dyn_arr_t *arr) {
    if (arr == NULL || arr->size <= 1) return;

    int first_val = arr->ptr[0];
    memmove(&arr->ptr[0], &arr->ptr[1], (arr->size - 1) * sizeof(int));
    arr->ptr[arr->size - 1] = first_val;
}

void dyn_arr_rotate_right(dyn_arr_t *arr) {
    if (arr == NULL || arr->size <= 1) return;

    int last_val = arr->ptr[arr->size - 1];
    memmove(&arr->ptr[1], &arr->ptr[0], (arr->size - 1) * sizeof(int));
    arr->ptr[0] = last_val;
}

static int compare_ints(const void *a, const void *b) {
    int arg1 = *(const int *)a;
    int arg2 = *(const int *)b;
    return (arg1 > arg2) - (arg1 < arg2);
}

bool dyn_arr_find_duplicates_sorted(dyn_arr_t *arr, dyn_arr_t *out_duplicates) {
    if (arr == NULL || out_duplicates == NULL || arr->size < 2) return false;

    qsort(arr->ptr, arr->size, sizeof(int), compare_ints);

    out_duplicates->size = 0;

    for (size_t i = 0; i < arr->size - 1; ++i) {
        if (arr->ptr[i] == arr->ptr[i + 1]) {
            if (out_duplicates->size == 0 ||
                out_duplicates->ptr[out_duplicates->size - 1] != arr->ptr[i])
            {
                if (!dyn_arr_append(out_duplicates, arr->ptr[i])) return false;
            }
        }
    }
    return true;
}