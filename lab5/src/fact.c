#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include <getopt.h>


int* break_to_parts(int value, int size);
void factorial(int begin, int end);

uint64_t total = 1;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

struct Period {
  int begin;
  int end;
};

void *ThreadFactorial(void *args) {
  struct Period *period = (struct Period *)args;

  factorial(period->begin, period->end);
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t pnum = -1;
  uint64_t mod = -1;

  while (true) {

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            k = atoi(optarg);
            if (k <= 0) {
              srand(time(NULL));
              k = rand() % 1000 + 1;
            }
            break;
          case 1:
            pnum = atoi(optarg);
            if (pnum <= 0) {
              pnum = 4;
            }
            break;
          case 2:
            mod = atoi(optarg);
            if (mod <= 0) {
              mod = rand() % 10 + 1;
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

  if (k > 20) {
    k = 20;
  }

  if (pnum > k) {
    pnum = k > 8 ? 8 : k;
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  if (k == -1 || pnum == -1 || mod == -1) {
    printf("Usage: %s --k \"num\" --pnum \"num\" --mod \"num\" \n",
           argv[0]);
    return 1;
  }

  int* parts = break_to_parts(k, pnum);
  int current_part = 0;

  struct Period *args = malloc(sizeof(struct Period) * pnum);
  pthread_t *threads = malloc(sizeof(pthread_t) * pnum);

  for (uint32_t i = 0; i < pnum; i++) {
    args[i].begin = current_part;
    args[i].end = current_part + parts[i];
    current_part += parts[i];

    if (pthread_create(&threads[i], NULL, ThreadFactorial, (void *)&args[i])) {
      printf("Error: pthread_create failed!\n");
      return 1;
    }
  }
  
  for (uint32_t i = 0; i < pnum; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("TOTAL MOD: %llu\n", (unsigned long long) total % (unsigned long long) mod);

  return 0;
}

void factorial(int begin, int end) {
    //printf("%d %d\n", begin, end);
    for (uint64_t i = begin + 1; i <= end; i++) {
        pthread_mutex_lock(&mut);
        total *= i;
        pthread_mutex_unlock(&mut);
    }
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
