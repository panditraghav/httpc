#include "array.h"

IntArray int_array_new(size_t capacity) {
  return (IntArray){.len = 0,
                    .capacity = capacity,
                    .internal_array = (int *)malloc(sizeof(int) * capacity)};
}

void int_array_push_back(IntArray *arr, int element) {
  if (arr->len == arr->capacity) {
    int *new_internal_array = (int *)malloc(sizeof(int) * arr->capacity * 2);
    memcpy(new_internal_array, arr->internal_array, arr->len * sizeof(int));
    free(arr->internal_array);
    arr->internal_array = new_internal_array;
    arr->capacity = 2 * arr->capacity;
  }
  arr->internal_array[arr->len] = element;
  arr->len = arr->len + 1;
}

int int_array_pop(IntArray *arr, int *popped) {
  if (arr->len == 0) {
    return 1;
  }

  *popped = arr->internal_array[arr->len - 1];
  arr->len = arr->len - 1;

  return 0;
}

//-----------------FloatArray------------------

FloatArray float_array_new(size_t capacity) {
  return (FloatArray){.len = 0,
                    .capacity = capacity,
                    .internal_array = (float *)malloc(sizeof(float) * capacity)};
}

void float_array_push_back(FloatArray *arr, float element) {
  if (arr->len == arr->capacity) {
    float *new_internal_array = (float *)malloc(sizeof(float) * arr->capacity * 2);
    memcpy(new_internal_array, arr->internal_array, arr->len * sizeof(int));
    free(arr->internal_array);
    arr->internal_array = new_internal_array;
    arr->capacity = 2 * arr->capacity;
  }
  arr->internal_array[arr->len] = element;
  arr->len = arr->len + 1;
}

int float_array_pop(FloatArray *arr, float *popped) {
  if (arr->len == 0) {
    return 1;
  }

  *popped = arr->internal_array[arr->len - 1];
  arr->len = arr->len - 1;

  return 0;
}
