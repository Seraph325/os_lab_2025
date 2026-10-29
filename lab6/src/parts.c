#include "parts.h"

#include <stdlib.h>

int* break_to_parts(int value, int size) {
  int* a = malloc(size * sizeof(int));
  int base_value = value / size;          
  int remainder = value % size; 
  for (int i = 0; i < size; i++) {
    *(a + i) = base_value + (i < remainder ? 1 : 0);
  }

  return a;
}
