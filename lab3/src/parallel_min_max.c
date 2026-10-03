#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int* break_to_parts(int value, int size);

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              srand(time(NULL));
              seed = rand();
            }
            // your code here
            // error handling
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              srand(time(NULL));
              array_size = rand() % 100;
            }
            // your code here
            // error handling
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              pnum = 4;
            }
            // your code here
            // error handling
            break;
          case 3:
            with_files = true;
            break;

          defalut:
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (pnum > array_size) {
    pnum = array_size > 8 ? 8 : array_size;
  }

  int* parts = break_to_parts(array_size, pnum);

  // printf("%d %d %d %d\n", seed, array_size, pnum, with_files);
  // return 0;

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" \n",
           argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  //test1(array, array_size, seed);
  int active_child_processes = 0;
  int fd[2];
  if (!with_files && pipe(fd) == -1) {
    printf("Failed to init pipe\n");
    return 1;
  }
  int current = 0;

  pid_t pid = fork();
  if (pid == 0) {
    char seed_str[32];
    char size_str[32];
    snprintf(seed_str, sizeof(seed_str), "%d", seed);
    snprintf(size_str, sizeof(size_str), "%d", array_size);

    execl("./sequential_min_max", "./sequential_min_max", seed_str, size_str, (char *)NULL);
    return 0;
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      // successful fork
      active_child_processes += 1;
      if (child_pid == 0) {
        // child process

        // parallel somehow
        struct MinMax min_max = GetMinMax(array, current, *(parts + i) + current);
        int* min_a_max = malloc(sizeof(int) * 2);
        min_a_max[0] = min_max.min;
        min_a_max[1] = min_max.max;

        if (with_files) {
          FILE* file;
          file = fopen("data.txt", "a");
          fwrite(min_a_max, sizeof(int), 2, file);
          fclose(file);
        } else {
          close(fd[0]);
          write(fd[1], min_a_max, sizeof(int) * 2);
          close(fd[1]);
        }

        return 0;
      }

    } else {
      printf("Fork failed!\n");
      return 1;
    }
    current += *(parts + i);
  }

  while (active_child_processes > 0) {
    wait(NULL);

    active_child_processes -= 1;
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  FILE* file;
  if (with_files) {
    file = fopen("data.txt", "r");
  }

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;
    
    int* min_a_max = malloc(sizeof(int) * 2); 

    if (with_files) {
      fread(min_a_max, sizeof(int), 2, file);
    } else {
      read(fd[0], min_a_max, sizeof(int) * 2);
    }
    if (min_a_max[0] < min) {
      min = min_a_max[0];
    }
    if (min_a_max[1] > max) {
      max = min_a_max[1];
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  if (with_files) {
    remove("data.txt");
    fclose(file);
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  fflush(NULL);
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

