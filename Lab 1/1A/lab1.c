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
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define RDONLY 0
#define WRONLY 1
#define COMMAND 2
#define VERBOSE 4

int* fd_table = NULL;
int fds = 0;

int to_pos_int(char* s) {
  char* rest = NULL;
  int val = strtol(s, &rest, 10);
  if (*rest != '\0') return -1;
  return val;
}

int exec_command(char** command_args) {
  pid_t child;

  // printf("Command: %s\n", command_args[3]);
  // for (int i = 4; command_args[i] != NULL; i++) printf("argv[%d]: %s\n", i -
  // 4, command_args[i]);

  for (int fd_i = 0; fd_i < 3; fd_i++) {
    int t_fd = to_pos_int(command_args[fd_i]);
    if (t_fd < 0 || t_fd >= fds) {
      fprintf(stderr, "--command error: invalid fd index %d given\n", t_fd);
      return -1;
    }
  }

  child = fork();
  if (child > 0) {
    return child;
  } else if (child == 0) {
    for (int fd_i = 0; fd_i < 3; fd_i++) {
      dup2(fd_table[to_pos_int(command_args[fd_i])], fd_i);
    }
    for (int fd_i = 0; fd_i < 3; fd_i++) {
      close(fd_table[to_pos_int(command_args[fd_i])]);
    }
    execvp(command_args[3], command_args + 3);
    fprintf(stderr, "Error during child execution of %s\n%s\n", command_args[3],
            strerror(errno));
    exit(1);
  } else {
    fprintf(stderr, "Error forking for execution of %s\n%s\n", command_args[3],
            strerror(errno));
    return -1;
  }
}

int is_long_arg(char* arg) {
  if (strlen(arg) < 2) return 0;

  if (arg[0] == '-' && arg[1] == '-') return 1;
  return 0;
}

int main(int argc, char** argv) {
  struct option long_options[] = {{"rdonly", required_argument, NULL, RDONLY},
                                  {"wronly", required_argument, NULL, WRONLY},
                                  {"command", required_argument, NULL, COMMAND},
                                  {"verbose", no_argument, NULL, VERBOSE},
                                  {0, 0, 0, 0}};

  int verbose_flag = 0;
  int exit_val = 0;

  int ret;
  while ((ret = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
    switch (ret) {
      case RDONLY: {
        if (verbose_flag) printf("--rdonly %s\n", optarg);

        fd_table = realloc(fd_table, (++fds) * sizeof(int));
        fd_table[fds - 1] = open(optarg, O_RDONLY);

        if (fd_table[fds - 1] == -1) {
          fprintf(stderr, "--input %s error: %s\n", optarg, strerror(errno));
          exit_val = 1;
        }
        break;
      }
      case WRONLY: {
        if (verbose_flag) printf("--wronly %s\n", optarg);

        fd_table = realloc(fd_table, (++fds) * sizeof(int));
        fd_table[fds - 1] = open(optarg, O_WRONLY);

        if (fd_table[fds - 1] == -1) {
          fprintf(stderr, "--output %s error: %s\n", optarg, strerror(errno));
          exit_val = 1;
        }
        break;
      }
      case COMMAND: {
        int n_args = 0;
        char** command_argv = NULL;

        for (optind--; optind < argc && !is_long_arg(argv[optind]); optind++) {
          command_argv = realloc(command_argv, sizeof(char*) * (++n_args));
          command_argv[n_args - 1] = argv[optind];
        }
        command_argv = realloc(command_argv, sizeof(char*) * (n_args + 1));
        command_argv[n_args] = NULL;

        // printf("n_args: %d\n", n_args);
        // for (int i = 0; command_argv[i] != NULL; i++) {
        //   printf("command_argv[%d]: %s\n", i, command_argv[i]);
        // }

        if (n_args < 4) {
          fprintf(stderr, "Not enough arguments given for --command\n");
          exit_val = 1;
          free(command_argv);
          break;
        }

        if (verbose_flag) {
          printf("--command");
          for (int i = 0; command_argv[i] != NULL; i++) {
            printf(" %s", command_argv[i]);
          }
          printf("\n");
        }

        if (exec_command(command_argv) == -1) {
          exit_val = 1;
        }
        free(command_argv);
        break;
      }
      case VERBOSE: {
        verbose_flag = 1;
        break;
      }
      case '?': {
        exit(1);
      }
    }
  }

  if (optind < argc) {
    for (int i = optind; i < argc; i++) {
      fprintf(stderr, "./lab0: unrecognized argument '%s'\n", argv[i]);
    }
    exit(1);
  }

  for (int fd_i = 0; fd_i < fds; fd_i++) {
    close(fd_table[fd_i]);
  }

  // for (int i = 0; i < argc; i++) {
  //   printf("%s ", argv[i]);
  // }
  // printf("\n");

  exit(exit_val);
}
