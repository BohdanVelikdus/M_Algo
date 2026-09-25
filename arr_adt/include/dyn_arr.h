#ifndef DYN_ARR_H
#define DYN_ARR_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int *ptr;
    size_t size;
    size_t capacity;
} dyn_arr_t;

// memory management
static bool dyn_arr_grow(dyn_arr_t *arr);
static bool dyn_arr_realloc(dyn_arr_t *arr, size_t total_size);

// constructor & destructor
bool dyn_arr_init(dyn_arr_t *arr, size_t capacity);
void dyn_arr_destroy(dyn_arr_t *arr);

// ADT funcs
void dyn_arr_display(const dyn_arr_t *arr);
bool dyn_arr_append(dyn_arr_t *arr, int value);
bool dyn_arr_insert(dyn_arr_t *arr, size_t index, int value);
// deleting element from pos and pop
bool dyn_arr_delete(dyn_arr_t *arr, size_t index, int* out_value);
// linear search for elem
bool dyn_arr_search(const dyn_arr_t *arr, int value);
bool dyn_arr_get(const dyn_arr_t *arr, size_t index, int *out_value);
bool dyn_arr_set(dyn_arr_t *arr, size_t index, int value);
void dyn_arr_reverse(dyn_arr_t *arr);
bool dyn_arr_is_sorted(const dyn_arr_t *arr);
// if array sorted, use binary search
bool dyn_arr_binary_search(const dyn_arr_t *arr, int value);
bool dyn_arr_merge(const dyn_arr_t *rhs, const dyn_arr_t *lhs, dyn_arr_t *result);

// num util
int max(const dyn_arr_t *arr);
int min(const dyn_arr_t *arr);
int dyn_arr_sum(const dyn_arr_t *arr);
double dyn_arr_avg(const dyn_arr_t *arr);

// set operations
bool dyn_arr_set_difference( const dyn_arr_t *lhs, const dyn_arr_t *rhs, dyn_arr_t *result);
bool dyn_arr_set_intersection( const dyn_arr_t *rhs, const dyn_arr_t *lhs, dyn_arr_t *result);
bool dyn_arr_set_union(const dyn_arr_t *lhs, const dyn_arr_t *rhs, dyn_arr_t *result);

// shift operations
void dyn_arr_shift_left(dyn_arr_t *arr);
void dyn_arr_shift_right(dyn_arr_t *arr);
void dyn_arr_rotate_left(dyn_arr_t *arr);
void dyn_arr_rotate_right(dyn_arr_t *arr);

// sorting
bool dyn_arr_find_duplicates_sorted(dyn_arr_t *arr, dyn_arr_t *out_duplicates);

#ifdef __cplusplus
}
#endif

#endif