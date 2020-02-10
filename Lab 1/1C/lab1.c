/*
NAME: Kevin Li
EMAIL: li.kevin512@gmail.com
ID: 123456789
*/

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define RDONLY 1
#define WRONLY 2
#define COMMAND 3
#define VERBOSE 4

#define RDWR 5
#define PIPE 6

#define WAIT 7
#define CHDIR 8
#define CLOSE 9
#define ABORT 10
#define CATCH 11
#define IGNORE 12
#define DEFAULT 13
#define PAUSE 14

#define PROFILE 15

#define NFFLAGS 11

int fflags[NFFLAGS] = {0};

int* write_fds = NULL;
int wfds;

int* fd_table = NULL;
int fds = 0;

int verbose_flag = 0;
int profile_flag = 0;

struct child_data {
  pid_t pid;
  char** args;
};

struct child_data* children = NULL;

int is_long_arg(char* arg) {
  if (strlen(arg) < 2) return 0;

  if (arg[0] == '-' && arg[1] == '-') return 1;
  return 0;
}

int to_pos_int(char* s) {
  char* rest = NULL;
  int val = strtol(s, &rest, 10);
  if (*rest != '\0') return -1;
  return val;
}

int exec_command(char** command_args) {
  pid_t child;

  int ioe[3];

  for (int fd_arg = 0; fd_arg < 3; fd_arg++) {
    int t_fd_i = to_pos_int(command_args[fd_arg]);
    if (t_fd_i < 0 || t_fd_i >= fds || fd_table[t_fd_i] == -1) {
      fprintf(stderr, "--command error: invalid fd index %d given\n", t_fd_i);
      return -1;
    }
    ioe[fd_arg] = t_fd_i;
  }

  child = fork();
  if (child > 0) {
    return child;
  } else if (child == 0) {
    for (int i = 0; i < 3; i++) {
      if (dup2(fd_table[ioe[i]], i) == -1) {
        fprintf(stderr, "error dup2ing");
      }
    }
    for (int i = 0; i < fds; i++) {
      close(fd_table[i]);
    }
    execvp(command_args[3], command_args + 3);
    fprintf(stderr, "Error during child execution of %s\n%s\n", command_args[3],
            strerror(errno));
    return -1;
  } else {
    fprintf(stderr, "Error forking for execution of %s\n%s\n", command_args[3],
            strerror(errno));
    return -1;
  }
}

int pop_fflags() {
  int res = 0;
  for (int i = 0; i < NFFLAGS; i++) {
    res |= fflags[i];
    fflags[i] = 0;
  }
  return res;
}

void signal_handler(int n) {
  fprintf(stderr, "%d caught", n);
  exit(n);
}

void print_time(char* option, struct rusage t) {
  struct timeval tu = t.ru_utime;
  struct timeval ts = t.ru_stime;
  double u_time = (double)tu.tv_sec + ((double)tu.tv_usec / 1000000);
  double s_time = (double)ts.tv_sec + ((double)ts.tv_usec / 1000000);
  printf("%s\t- User time: %f\t- System time: %f\n", option, u_time, s_time);
}

void print_time_diff(char* option, struct rusage t1, struct rusage t2) {
  struct timeval t1u = t1.ru_utime;
  struct timeval t1s = t1.ru_stime;
  struct timeval t2u = t2.ru_utime;
  struct timeval t2s = t2.ru_stime;
  double u_time = ((double)t2u.tv_sec - (double)t1u.tv_sec) +
                  (((double)t2u.tv_usec - (double)t1u.tv_usec) / 1000000);
  double s_time = ((double)t2s.tv_sec - (double)t1s.tv_sec) +
                  (((double)t2s.tv_usec - (double)t1s.tv_usec) / 1000000);
  printf("%s\t- User time: %f\t- System time: %f\n", option, u_time, s_time);
}

int main(int argc, char** argv) {
  int total_profile = 0;
  struct option long_options[] = {
      {"rdonly", required_argument, NULL, RDONLY},
      {"wronly", required_argument, NULL, WRONLY},
      {"rdwr", required_argument, NULL, RDWR},
      {"pipe", no_argument, NULL, PIPE},

      {"command", required_argument, NULL, COMMAND},
      {"wait", no_argument, NULL, WAIT},

      {"chdir", required_argument, NULL, CHDIR},
      {"close", required_argument, NULL, CLOSE},
      {"abort", no_argument, NULL, ABORT},
      {"catch", required_argument, NULL, CATCH},
      {"ignore", required_argument, NULL, IGNORE},
      {"default", required_argument, NULL, DEFAULT},
      {"pause", no_argument, NULL, PAUSE},

      {"verbose", no_argument, &verbose_flag, 1},

      {"append", no_argument, fflags, O_APPEND},
      {"cloexec", no_argument, fflags + 1, O_CLOEXEC},
      {"creat", no_argument, fflags + 2, O_CREAT},
      {"directory", no_argument, fflags + 3, O_DIRECTORY},
      {"dsync", no_argument, fflags + 4, O_DSYNC},
      {"excl", no_argument, fflags + 5, O_EXCL},
      {"nofollow", no_argument, fflags + 6, O_NOFOLLOW},
      {"nonblock", no_argument, fflags + 7, O_NONBLOCK},
      {"rsync", no_argument, fflags + 8, O_RSYNC},
      {"sync", no_argument, fflags + 9, O_SYNC},
      {"trunc", no_argument, fflags + 10, O_TRUNC},

      {"profile", no_argument, NULL, PROFILE},
      {"total-profile", no_argument, &total_profile, 1},
      {0, 0, 0, 0}};

  int exit_val = 0;
  int max_child_exit_val = -1;

  int n_children = 0;
  int children_reaped = 0;

  char* pwd = get_current_dir_name();

  struct rusage t1, t2;
  struct rusage last_child_usage, child_usage;

  char* opt = NULL;
  if (argc > 1) opt = argv[1];

  int ret;
  while ((ret = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
    if (profile_flag) {
      getrusage(RUSAGE_SELF, &t1);
    }

    switch (ret) {
      case RDONLY:
      case WRONLY:
      case RDWR: {
        int file_flags = pop_fflags();
        char* open_mode = NULL;

        if (ret == RDONLY) {
          file_flags |= O_RDONLY;
        } else if (ret == WRONLY) {
          file_flags |= O_WRONLY;
        } else {
          file_flags |= O_RDWR;
        }

        if (verbose_flag) {
          printf("%s %s\n", opt, optarg);
          fflush(stdout);
        }

        fd_table = realloc(fd_table, (++fds) * sizeof(int));
        fd_table[fds - 1] = open(optarg, file_flags, 0666);

        if (fd_table[fds - 1] == -1) {
          fprintf(stderr, "%s %s error: %s\n", open_mode, optarg,
                  strerror(errno));
          exit_val = 1;
        }
        break;
      }
      case PIPE: {
        int file_flags = pop_fflags();
        if (verbose_flag) {
          printf("%s\n", opt);
          fflush(stdout);
        }

        int mpipe[2];
        fds += 2;
        wfds++;
        fd_table = realloc(fd_table, fds * sizeof(int));
        write_fds = realloc(write_fds, wfds * sizeof(int));
        if (pipe2(mpipe, file_flags) == -1) {
          fprintf(stderr, "--pipe %s error: %s\n", optarg, strerror(errno));
          exit_val = 1;
        }
        fd_table[fds - 2] = mpipe[0];
        fd_table[fds - 1] = mpipe[1];
        write_fds[wfds - 1] = fds - 1;
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

        if (n_args < 4) {
          fprintf(stderr, "--command error: Not enough arguments given\n");
          exit_val = 1;
          free(command_argv);
          break;
        }

        if (verbose_flag) {
          printf("%s ", opt);
          for (int i = 0; command_argv[i] != NULL; i++) {
            printf(" %s", command_argv[i]);
          }
          printf("\n");
        }

        int child;
        if ((child = exec_command(command_argv)) > 0) {
          children = (struct child_data*)realloc(
              children, (++n_children) * sizeof(struct child_data));
          children[n_children - 1].pid = child;
          children[n_children - 1].args = command_argv;
        } else {
          exit_val = 1;
        }
        break;
      }
      case ABORT: {
        if (verbose_flag) {
          printf("%s\n", opt);
          fflush(stdout);
        }
        int* eresting = NULL;
        *eresting = 42;
        break;
      }
      case CATCH: {
        if (verbose_flag) {
          printf("%s %s\n", opt, optarg);
          fflush(stdout);
        }
        int n = to_pos_int(optarg);
        signal(n, signal_handler);
        break;
      }
      case DEFAULT: {
        if (verbose_flag) {
          printf("%s %s\n", opt, optarg);
          fflush(stdout);
        }
        int n = to_pos_int(optarg);
        signal(n, SIG_DFL);
        break;
      }
      case IGNORE: {
        if (verbose_flag) {
          printf("%s %s\n", opt, optarg);
          fflush(stdout);
        }
        int n = to_pos_int(optarg);
        signal(n, SIG_IGN);
        break;
      }
      case PAUSE: {
        if (verbose_flag) {
          printf("%s\n", opt);
          fflush(stdout);
        }
        pause();
        break;
      }
      case CLOSE: {
        if (verbose_flag) {
          printf("%s %s\n", opt, optarg);
          fflush(stdout);
        }
        int fd = to_pos_int(optarg);

        if (fd < 0 || fd >= fds || fd_table[fd] == -1) {
          fprintf(stderr, "--close error: invalid fd index %d given\n", fd);
          exit_val = 1;
        } else {
          close(fd_table[fd]);
          fd_table[fd] = -1;
        }
        break;
      }
      case WAIT: {
        if (verbose_flag) {
          printf("%s\n", opt);
          fflush(stdout);
        }

        for (int i = 0; i < wfds; i++) {
          int tfd = write_fds[i];
          if (tfd != -1) {
            close(fd_table[tfd]);
            fd_table[tfd] = -1;
          }
        }

        while (children_reaped < n_children) {
          int child_status;
          if (profile_flag) getrusage(RUSAGE_CHILDREN, &last_child_usage);

          pid_t child = wait(&child_status);

          children_reaped++;

          int child_i;
          for (child_i = 0; children[child_i].pid != child; child_i++)
            ;

          int c_status;
          if (WIFEXITED(child_status)) {
            c_status = WEXITSTATUS(child_status);
            printf("exit %d ", c_status);
          } else if (WIFSIGNALED(child_status)) {
            c_status = WTERMSIG(child_status);
            printf("signal %d ", c_status);
            c_status += 128;
          }
          max_child_exit_val =
              (max_child_exit_val > c_status) ? max_child_exit_val : c_status;

          for (int i = 3; children[child_i].args[i] != NULL; i++) {
            printf("%s ", children[child_i].args[i]);
          }
          printf("\n");

          if (profile_flag) {
            getrusage(RUSAGE_CHILDREN, &child_usage);
            print_time_diff("Child time:", last_child_usage, child_usage);
          }
        }
        break;
      }
      case CHDIR: {
        if (verbose_flag) {
          printf("%s %s\n", opt, optarg);
          fflush(stdout);
        }

        int path_len = strlen(pwd) + strlen(optarg) + 2;

        char* full_path = malloc(path_len * sizeof(char));
        strcpy(full_path, pwd);
        full_path[strlen(pwd)] = '/';
        full_path[strlen(pwd) + 1] = '\0';
        strcat(full_path, optarg);

        if (chdir(full_path) == -1) {
          fprintf(stderr, "--chdir %s error: %s\n", optarg, strerror(errno));
          exit_val = 1;
          free(full_path);
        } else {
          free(pwd);
          pwd = full_path;
        }

        break;
      }
      case PROFILE: {
        if (verbose_flag) {
          printf("%s\n", opt);
          fflush(stdout);
        }

        profile_flag = 2;
        break;
      }
      case '?': {
        exit_val = 1;
        break;
      }
    }
    if (profile_flag == 1) {
      getrusage(RUSAGE_SELF, &t2);
      print_time_diff(opt, t1, t2);
    } else if (profile_flag == 2) {
      profile_flag = 1;
    }

    if (optind < argc)
      opt = argv[optind];
    else
      opt = NULL;
  }

  for (int i = 0; i < n_children; i++) {
    free(children[i].args);
  }

  for (int fd_i = 0; fd_i < fds; fd_i++) {
    // printf("fd_table[%d] = %d\n", fd_i, fd_table[fd_i]);
    if (fd_table[fd_i] != -1) close(fd_table[fd_i]);
  }

  free(write_fds);
  free(pwd);
  free(fd_table);
  free(children);

  if (optind < argc) {
    for (int i = optind; i < argc; i++) {
      fprintf(stderr, "./lab0: unrecognized argument '%s'\n", argv[i]);
    }
    exit_val = 1;
  }

  if (total_profile) {
    getrusage(RUSAGE_SELF, &t2);
    print_time("Total time taken", t2);

    getrusage(RUSAGE_CHILDREN, &child_usage);
    print_time("Total child time", child_usage);
  }

  exit_val = (max_child_exit_val > exit_val) ? max_child_exit_val : exit_val;

  exit(exit_val);
}
