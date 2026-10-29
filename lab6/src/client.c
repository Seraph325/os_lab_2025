#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "parts.h"

void read_addr_till(FILE* file, char* ptr, char till);
void clear_buffer(char* buffer);
//int* break_to_parts(int value, int size);

struct Server {
  char ip[255];
  int port;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
  uint64_t result = 0;
  a = a % mod;
  while (b > 0) {
    if (b % 2 == 1)
      result = (result + a) % mod;
    a = (a * 2) % mod;
    b /= 2;
  }

  return result % mod;
}

bool ConvertStringToUI64(const char *str, uint64_t *val) {
  // char* copy = str;
  // for (int i = 0; i < strlen(copy); i++) {
  //   printf("%c", copy[i]);
  // }
  char *end = NULL;
  unsigned long long i = strtoull(str, &end, 10);
  if (errno == ERANGE) {
    fprintf(stderr, "Out of uint64_t range: %s\n", str);
    return false;
  }

  if (errno != 0) {
    return false;
  }
  *val = i;
  return true;
}

int main(int argc, char **argv) {
  uint64_t k = -1;
  uint64_t mod = -1;
  char servers[255] = {'\0'}; // TODO: explain why 255
  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"k", required_argument, 0, 0},
                                      {"mod", required_argument, 0, 0},
                                      {"servers", required_argument, 0, 0},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "", options, &option_index);

    if (c == -1)
      break;

    switch (c) {
    case 0: {
      switch (option_index) {
      case 0:
        ConvertStringToUI64(optarg, &k);
        // TODO: your code here
        break;
      case 1:
        ConvertStringToUI64(optarg, &mod);
        // TODO: your code here
        break;
      case 2:
        // TODO: your code here
        memcpy(servers, optarg, strlen(optarg));
        break;
      default:
        printf("Index %d is out of options\n", option_index);
      }
    } break;

    case '?':
      printf("Arguments error\n");
      break;
    default:
      fprintf(stderr, "getopt returned character code 0%o?\n", c);
    }
  }

  if (k == -1 || mod == -1 || !strlen(servers)) {
    fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n",
            argv[0]);
    return 1;
  }

  // TODO: for one server here, rewrite with servers from file
  FILE* file;
  char ip[255], port[8];

  file = fopen(servers, "r");
  if (file == NULL) {
    exit(1);
  }
  char buffer[128];
  read_addr_till(file, buffer, '\n');
  unsigned int servers_num;
  ConvertStringToUI64(buffer, &servers_num);
  
  struct Server *to = malloc(sizeof(struct Server) * servers_num);
  for (int i = 0; i < servers_num; i++) {
    read_addr_till(file, ip, ':');
    read_addr_till(file, port, '\n');
    uint64_t port_val = 0;
    if (!ConvertStringToUI64(port, &port_val)) exit(1);
    to[i].port = (int)port_val;
    memcpy(to[i].ip, ip, 255);

    clear_buffer(ip);
    clear_buffer(port);
  }

  fclose(file);

  if (servers_num > k) {
    servers_num = k;
  }

  int* parts = break_to_parts(k, servers_num);
  //for (int i = 0; i < servers_num; i++) printf("%d ", *(parts + i));
  
  uint64_t current = 0;
  uint64_t result = 1;

  // TODO: work continiously, rewrite to make parallel
  for (int i = 0; i < servers_num; i++) {
    struct hostent *hostname = gethostbyname(to[i].ip);
    if (hostname == NULL) {
      fprintf(stderr, "gethostbyname failed with %s\n", to[i].ip);
      exit(1);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(to[i].port);
    memcpy(&server.sin_addr, hostname->h_addr_list[0], sizeof(struct in_addr));


    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
      fprintf(stderr, "Socket creation failed!\n");
      exit(1);
    }

    if (connect(sck, (struct sockaddr *)&server, sizeof(server)) < 0) {
      fprintf(stderr, "Connection failed\n");
      exit(1);
    }

    // TODO: for one server
    // parallel between servers
    uint64_t begin = current;
    uint64_t end = current + parts[i];
    current += (uint64_t) parts[i];

    char* task = malloc(sizeof(uint64_t) * 3);
    memcpy(task, &begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &mod, sizeof(uint64_t));

    if (send(sck, task, 3 * sizeof(uint64_t), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        exit(1);
    }
    free(task);

    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
      fprintf(stderr, "Recieve failed\n");
      exit(1);
    }

    // TODO: from one server
    // unite results
    uint64_t answer = 0;
    memcpy(&answer, response, sizeof(uint64_t));
    result *= answer;

    close(sck);
  }
  free(to);

  printf("answer: %llu\n", result);

  return 0;
}

void read_addr_till(FILE* file, char* ptr, char till) {
  char* ptr_copy = ptr;
  char read_c[1], c;
  size_t read_count = fread(read_c, sizeof(char), 1, file);

  do {
    c = read_c[0];

    if (c == till) {
      break;
    }

    *(ptr_copy) = c;
    ptr_copy++;
    
    read_count = fread(read_c, sizeof(char), 1, file);
  } while (read_count != 0);
  
  *(ptr_copy) = '\0';
}

void clear_buffer(char* buffer) {
  char* buffer_copy = buffer;
  while(*(buffer_copy) != '\0') {
    *(buffer_copy) = '\0';
    buffer_copy++;
  }
}