/*
NAME: Kevin Li
EMAIL: li.kevin512@gmail.com
ID: 123456789
*/

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INPUT 0
#define OUTPUT 1
#define SEGFAULT 2
#define CATCH 3
#define DUMP_CORE 4

void forced_segfault() {
  int* eresting_pointer = NULL;
  *eresting_pointer = 42;
}

void sigsegv_handler() {
  char* message = "Segmentation fault caught. Exiting...";
  char newline = '\n';
  write(STDERR_FILENO, message, 37);
  write(STDERR_FILENO, &newline, 1);
  exit(4);
}

int main(int argc, char** argv) {
  struct option long_options[] = {{"input", required_argument, NULL, INPUT},
                                  {"output", required_argument, NULL, OUTPUT},
                                  {"segfault", no_argument, NULL, SEGFAULT},
                                  {"catch", no_argument, NULL, CATCH},
                                  {"dump-core", no_argument, NULL, DUMP_CORE},
                                  {0, 0, 0, 0}};

  int ret;

  int catch = 0;
  int seg_fault = 0;

  while ((ret = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
    switch (ret) {
      case INPUT: {
        int input_fd = open(optarg, O_RDONLY);
        if (input_fd == -1) {
          fprintf(stderr, "--input error opening input file \"%s\"\n%s\n",
                  optarg, strerror(errno));
          exit(2);
        }
        if(dup2(input_fd, 0) == -1) {
          fprintf(stderr, "--input error when calling dup2(%d, 0) to replace fd 0 (stdin) with fd %d\n%s\n",
                  input_fd, input_fd, strerror(errno));
          exit(2);
        }
        break;
      }

      case OUTPUT: {
        int output_fd = creat(optarg, 0700);
        if (output_fd == -1) {
          fprintf(stderr, "--output error opening output file \"%s\"\n%s\n",
                  optarg, strerror(errno));
          exit(3);
        }
        if(dup2(output_fd, 1) == -1) {
          fprintf(stderr, "--output error when calling dup2(%d, 0) to replace fd 1 (stdout) with fd %d\n%s\n",
                  output_fd, output_fd, strerror(errno));
          exit(2);
        }
        break;
      }

      case SEGFAULT: {
        seg_fault = 1;
        break;
      }

      case CATCH: {
        catch = 1;
        break;
      }

      case DUMP_CORE: {
        catch = 0;
        break;
      }

      case '?': {
        exit(1);
        break;
      }
    }
  }

  if(catch) {
    signal(SIGSEGV, sigsegv_handler);
  }

  if(seg_fault) {
    forced_segfault();
  }

  if (optind < argc) {
    for (int i = optind; i < argc; i++) {
      fprintf(stderr, "./lab0: unrecognized argument '%s'\n", argv[i]);
    }
    exit(3);
  }

  char character;
  int rd_status, wr_status;
  while ((rd_status = read(0, &character, 1)) > 0) {
    wr_status = write(1, &character, 1);
    if (wr_status == -1) {
      fprintf(stderr, "Error writing to FD 1\n%s\n", strerror(errno));
      exit(3);
    }
  }

  if (rd_status == -1) {
    fprintf(stderr, "Error reading from FD 0\n%s\n", strerror(errno));
    exit(2);
  }

  close(0);
  close(1);

  exit(0);
}