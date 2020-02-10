/*
NAME: Kevin Li
EMAIL: li.kevin512@gmail.com
ID: 123456789
*/

#define _GNU_SOURCE

#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define THREADS 1
#define ITERATIONS 2
#define YIELD 3
#define SYNC 4

long long counter;
int opt_yield;
pthread_mutex_t m_lock;
volatile int s_lock;

// Stores which lock to use
char lock_type[2] = "";

// add function as given and described
void add(long long* pointer, long long value) {
  long long sum = *pointer + value;
  if (opt_yield) sched_yield();
  *pointer = sum;
}

// Mutex protected add
void mutex_add(long long* pointer, long long value) {
  pthread_mutex_lock(&m_lock);
  add(pointer, value);
  pthread_mutex_unlock(&m_lock);
}

// Spin lock protected add
void spin_add(long long* pointer, long long value) {
  while (__sync_lock_test_and_set(&s_lock, 1) == 1) continue;
  add(pointer, value);
  __sync_lock_release(&s_lock);
}

// Compare and swap add
void cas_add(long long* pointer, long long value) {
  long long old, new;
  do {
    old = *pointer;
    new = old + value;
  } while (old != __sync_val_compare_and_swap(pointer, old, new));
}

// Wrapper function to determine which add to use (i.e. which lock)
// and to allow it to be passed to pthread_create()
void* add_wrapper(void* arg) {
  // Uses a function pointer to know which add function to use
  void (*add_func)(long long*, long long) = add;
  switch (lock_type[0]) {
    case 'm':
      add_func = mutex_add;
      break;
    case 's':
      add_func = spin_add;
      break;
    case 'c':
      add_func = cas_add;
      break;
  }

  int n = *(int*)arg;
  for (int i = 0; i < n; i++) add_func(&counter, 1);
  for (int i = 0; i < n; i++) add_func(&counter, -1);
  return NULL;
}

// Reports erros and exits with code 1
void err(char* error) {
  fprintf(stderr, "%s: %s\n", error, strerror(errno));
  exit(1);
}

// Gets the name for the csv output based on global vars
void get_name(char* name) {
  strcpy(name, "add");
  if (opt_yield) strcat(name, "-yield");
  strcat(name, "-");
  if (strcmp(lock_type, "") == 0)
    strcat(name, "none");
  else
    strcat(name, lock_type);
}

int main(int argc, char* argv[]) {
  int nthreads = 1;
  int iterations = 1;

  // Grabs and interprets options
  // --threads sets number of threads to use
  // --iterations sets number of iterations
  // --yield decides whether to yield or not
  // --sync defines which lock
  static const struct option long_options[] = {
      {"threads", required_argument, NULL, THREADS},
      {"iterations", required_argument, NULL, ITERATIONS},
      {"yield", no_argument, &opt_yield, 1},
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
      case SYNC: {
        if (strlen(optarg) > 1 ||
            (optarg[0] != 'm' && optarg[0] != 's' && optarg[0] != 'c')) {
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

  // Runs and times the tests
  struct timespec start, end;
  pthread_t* threads = (pthread_t*)malloc(nthreads * sizeof(pthread_t));
  if (threads == NULL) err("malloc() error creating thread space");

  if (clock_gettime(CLOCK_MONOTONIC, &start) == -1)
    err("clock_gettime() failure getting start time");

  for (int t = 0; t < nthreads; t++)
    if (pthread_create(&threads[t], NULL, add_wrapper, &iterations) != 0)
      err("pthread_create() error");

  for (int t = 0; t < nthreads; t++)
    if (pthread_join(threads[t], NULL)) err("pthread_join() failure");

  if (clock_gettime(CLOCK_MONOTONIC, &end) == -1)
    err("clock_gettime() failure getting end time");

  // Sets up vars for and prints the csv output
  long long time_ns =
      (end.tv_sec - start.tv_sec) * 1000000000 + end.tv_nsec - start.tv_nsec;

  char name[32];
  get_name(name);

  long long ops = (long long)nthreads * iterations * 2;
  long long op_t = time_ns / ops;

  printf("%s,%d,%d,%lld,%lld,%lld,%lld\n", name, nthreads, iterations, ops,
         time_ns, op_t, counter);
  free(threads);
  exit(0);
}