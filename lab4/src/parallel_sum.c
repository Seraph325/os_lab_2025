#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <getopt.h>
#include <pthread.h>

int* break_to_parts(int value, int size);

struct SumArgs {
  int *array;
  int begin;
  int end;
};

uint64_t Sum(const struct SumArgs *args) {
  uint64_t sum = 0;
  for (int i = args->begin; i < args->end; i++) {
    sum += args->array[i];
  }
  return sum;
}

void *ThreadSum(void *args) {
  
  struct SumArgs *sum_args = (struct SumArgs *)args;
  return (void *)(uint64_t)Sum(sum_args);
}

int main(int argc, char **argv) {
  /*
   *  TODO:
   *  threads_num by command line arguments
   *  array_size by command line arguments
   *	seed by command line arguments
   */

  uint32_t threads_num = -1;
  uint32_t array_size = -1;
  uint32_t seed = -1;

    while (1) {
      static struct option options[] = {{"threads_num", required_argument, 0, 0},
                                        {"array_size", required_argument, 0, 0},
                                        {"seed", required_argument, 0, 0},
                                        {0, 0, 0, 0}};

      int option_index = 0;
      int c = getopt_long(argc, argv, "f", options, &option_index);

      if (c == -1) break;

      switch (c) {
        case 0:
          switch (option_index) {
            case 0:
              threads_num = atoi(optarg);
              if (threads_num <= 0) {
                threads_num = 4;
              }
              break;
            case 1:
              array_size = atoi(optarg);
              if (array_size <= 0) {
                srand(time(NULL));
                array_size = rand() % 100;
              }
              break;
            case 2:
              seed = atoi(optarg);
              if (seed <= 0) {
                srand(time(NULL));
                seed = rand();
              }
              break;

            defalut:
              printf("Index %d is out of options\n", option_index);
          }
          break;

        case '?':
          break;

        default:
          printf("getopt returned character code 0%o?\n", c);
      }
    }

    if (threads_num > array_size) {
      threads_num = array_size > 8 ? 8 : array_size;
    }


    if (optind < argc) {
      printf("Has at least one no option argument\n");
      return 1;
  } 

    if (threads_num == -1 || array_size == -1 || seed == -1) {
      printf("Usage: %s --threads_num \"num\" --array_size \"num\" --seed \"num\" \n", argv[0]);
      return 1;
    }


  /*
   * TODO:
   * your code here
   * Generate array here
   */

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  
  int* parts = break_to_parts(array_size, threads_num);
  int current_part = 0;

  pthread_t *threads = malloc(sizeof(pthread_t) * threads_num);
  struct SumArgs *args = malloc(sizeof(struct SumArgs) * threads_num);

  for (uint32_t i = 0; i < threads_num; i++) {
    args[i].array = array;
    args[i].begin = current_part;
    args[i].end = current_part + parts[i];
    current_part += parts[i];

    if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }
  

  uint64_t total_sum = 0;
  for (uint32_t i = 0; i < threads_num; i++) {
    uint64_t sum = 0;
    pthread_join(threads[i], (void **)&sum);
    total_sum += sum;
  }

  free(array);
  printf("Total: %d\n", total_sum);
  return 0;
}

int* break_to_parts(int value, int size) {
  int* a = malloc(size * sizeof(int));
  int base_value = value / size;          
  int remainder = value % size; 
  for (int i = 0; i < size; i++) {
    *(a + i) = base_value + (i < remainder ? 1 : 0);
  }

  return a;
}
