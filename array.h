#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct IntArray {
  size_t len; // Last element will be at len - 1 given that len > 0.
  size_t capacity;
  int *internal_array;
} IntArray;

IntArray int_array_new(size_t capacity);
void int_array_push_back(IntArray *arr, int element);
int int_array_pop(IntArray *arr, int *popped);

typedef struct FloatArray {
  size_t len; // Last element will be at len - 1 given that len > 0.
  size_t capacity;
  float *internal_array;
} FloatArray;

FloatArray float_array_new(size_t capacity);
void float_array_push_back(FloatArray *arr, float element);
int float_array_pop(FloatArray *arr, float *popped);
