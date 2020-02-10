/*
NAME: Kevin Li
EMAIL: li.kevin512@gmail.com
ID: 123456789
*/

#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "SortedList.h"

#define THREADS 1
#define ITERATIONS 2
#define YIELD 3
#define SYNC 4

int opt_yield;

pthread_mutex_t m_lock;
volatile int s_lock;

// Stores which lock to use
char lock_type[2] = "";

int nthreads = 1;
int iterations = 1;

SortedList_t* list;

// multidimensional array of list elements
// makes future manipualation very convenient
// First dimension points is thread number
// Second dimension is element number
SortedListElement_t** elements;

// Used as space dedicated for the keys of each element
char* keys;

// Initializes and sets up elements array
void create_initialize_elements() {
  elements = malloc(nthreads * sizeof(SortedListElement_t*));
  keys = malloc(nthreads * iterations * sizeof(char));
  for (int t = 0; t < nthreads; t++) {
    elements[t] = malloc(iterations * sizeof(SortedListElement_t));
    for (int e = 0; e < iterations; e++) {
      keys[t * e + e] = (char)('a' + rand() % 26);
      elements[t][e].key = keys + ((t * e) + e);
    }
  }
}

// When corruption is found, prints a diagnostic and
// exits with status 2
void found_corruption(char* msg) {
  fprintf(stderr, "Corruption found; %s\n", msg);
  exit(2);
}

// When a system or argument error occurs, prints relevant
// information and exists with status 1
void err(char* error) {
  fprintf(stderr, "%s: %s\n", error, strerror(errno));
  exit(1);
}

// Locks the list using the specified lock (or none)
void lock_list() {
  if (lock_type[0] == 'm') pthread_mutex_lock(&m_lock);
  if (lock_type[0] == 's')
    while (__sync_lock_test_and_set(&s_lock, 1))
      while (s_lock) continue;
}

// Unlocks the list using the specified lock (or none)
void unlock_list() {
  if (lock_type[0] == 'm') pthread_mutex_unlock(&m_lock);
  if (lock_type[0] == 's') __sync_lock_release(&s_lock);
}

// Processes the list as specified
// Specifically inserts, counts, and removes elements
void* process_list(void* t_id) {
  int tid = *(int*)t_id;

  // inserting elements
  for (int e = 0; e < iterations; e++) {
    lock_list();
    SortedList_insert(list, (elements[tid]) + e);
    unlock_list();
  }

  // checking length and looking for corruption
  lock_list();
  if (SortedList_length(list) < 0) found_corruption("could not get length");
  unlock_list();

  // looking up and deleting everything that was inserted
  for (int e = 0; e < iterations; e++) {
    lock_list();
    SortedListElement_t* element =
        SortedList_lookup(list, elements[tid][e].key);
    if (element == NULL) found_corruption("could not lookup inserted element");
    if (SortedList_delete(element) == 1)
      found_corruption("could not delete element");
    unlock_list();
  }

  return NULL;
}

// Gets the name for the csv output based on global vars
void get_name(char* name) {
  strcpy(name, "list-");

  if (opt_yield & INSERT_YIELD) strcat(name, "i");
  if (opt_yield & DELETE_YIELD) strcat(name, "d");
  if (opt_yield & LOOKUP_YIELD) strcat(name, "l");
  if (name[strlen(name) - 1] == '-') strcat(name, "none");

  strcat(name, "-");
  if (strcmp(lock_type, "") == 0)
    strcat(name, "none");
  else
    strcat(name, lock_type);
}

// Catches segmentation faults and handles them in a
// streamlined fashion. It does count as corruption
void sigsegv_handler(int n) {
  if (n == SIGSEGV) found_corruption("segmentation fault");
}

int main(int argc, char* argv[]) {
  // Grabs and interprets options
  // --threads sets number of threads to use
  // --iterations sets number of iterations
  // --yield decides where to yield
  // --sync defines which lock
  static const struct option long_options[] = {
      {"threads", required_argument, NULL, THREADS},
      {"iterations", required_argument, NULL, ITERATIONS},
      {"yield", required_argument, NULL, YIELD},
      {"sync", required_argument, NULL, SYNC},
      {0, 0, 0, 0}};

  int ret;
  while ((ret = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
    switch (ret) {
      case THREADS: {
        nthreads = atoi(optarg);
        break;
      }
      case ITERATIONS: {
        iterations = atoi(optarg);
        break;
      }
      case YIELD: {
        for (int i = 0; optarg[i] != '\0'; i++) {
          switch (optarg[i]) {
            case 'i':
              opt_yield |= INSERT_YIELD;
              break;
            case 'd':
              opt_yield |= DELETE_YIELD;
              break;
            case 'l':
              opt_yield |= LOOKUP_YIELD;
              break;
            default:
              fprintf(stderr, "--yield error: Invalid argument %c\n",
                      optarg[i]);
              exit(1);
          }
        }
        break;
      }
      case SYNC: {
        if (strlen(optarg) > 1 || (optarg[0] != 'm' && optarg[0] != 's')) {
          fprintf(stderr, "--sync=%s error: invalid argument %s\n", optarg,
                  optarg);
          exit(1);
        }
        strcpy(lock_type, optarg);
        break;
      }
      case '?': {
        exit(1);
        break;
      }
    }
  }

  // Initializes and sets up stage for list operations
  signal(SIGSEGV, sigsegv_handler);

  list = (SortedList_t*)malloc(sizeof(SortedList_t));
  list->next = list->prev = list;

  create_initialize_elements();

  pthread_t* threads = (pthread_t*)malloc(nthreads * sizeof(pthread_t));


  // Does operations and times it
  struct timespec start, end;
  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
    err("clock_gettime() failure getting start time");

  int* tids = (int*)malloc(nthreads * sizeof(int));
  for (int t = 0; t < nthreads; t++) {
    tids[t] = t;
    if (pthread_create(&threads[t], NULL, process_list, tids + t) != 0)
      err("pthread_create() error");
  }

  for (int t = 0; t < nthreads; t++)
    if (pthread_join(threads[t], NULL)) err("pthread_join() failure");

  if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
    err("clock_gettime() failure getting end time");

  // Looks for corruption one last time
  if (SortedList_length(list) != 0)
    found_corruption("length is nonzero at end");

  // Sets up vars for and prints the csv output
  long long time_ns =
      (end.tv_sec - start.tv_sec) * 1000000000 + end.tv_nsec - start.tv_nsec;

  char name[32];
  get_name(name);

  long long ops = (long long)nthreads * iterations * 3;
  long long op_t = time_ns / ops;

  printf("%s,%d,%d,%d,%lld,%lld,%lld\n", name, nthreads, iterations, 1, ops,
         time_ns, op_t);

  free(threads);
  free(tids);

  free(list);

  for (int t = 0; t < nthreads; t++) free(elements[t]);
  free(elements);
  free(keys);

  exit(0);
}