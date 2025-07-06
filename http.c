#include "array.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() {
  IntArray arr = int_array_new(2);
  int_array_push_back(&arr, 10);
  int_array_push_back(&arr, 20);
  int_array_push_back(&arr, 33);
  int_array_push_back(&arr, 22);
  int_array_push_back(&arr, 1234);

  for (size_t i = 0; i < arr.len; i++) {
    printf("%d\n", arr.internal_array[i]);
  }

  printf("Length: %zu\n", arr.len);
  printf("Capacity: %zu\n", arr.capacity);

  int popped;
  int pop_error;

  pop_error = int_array_pop(&arr, &popped);
  printf("popped: %d, pop_error: %d\n", popped, pop_error);

  pop_error = int_array_pop(&arr, &popped);
  printf("popped: %d, pop_error: %d\n", popped, pop_error);

  pop_error = int_array_pop(&arr, &popped);
  printf("popped: %d, pop_error: %d\n", popped, pop_error);

  for (size_t i = 0; i < arr.len; i++) {
    printf("%d\n", arr.internal_array[i]);
  }

  printf("Length: %zu\n", arr.len);
  printf("Capacity: %zu\n", arr.capacity);
}
