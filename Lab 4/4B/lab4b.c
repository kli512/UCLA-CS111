// NAME: Kevin Li
// EMAIL: li.kevin512@gmail.com
// ID: 123456789
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define SCALE 1
#define PERIOD 2
#define LOG 3

int period = 1;
char scale = 'F';
FILE* log_file = NULL;
int running = 1;
int stopped = 0;

mraa_aio_context temp_sens;
mraa_gpio_context but;

void err(char* error) {
  fprintf(stderr, "%s: %s\n", error, strerror(errno));
  exit(1);
}

int is_prefix(char* pre, char* s) {
  if (strlen(pre) > strlen(s)) return 0;
  return !memcmp(pre, s, strlen(pre));
}

void read_command(char* command) {
  if (!strcmp(command, "SCALE=F\n"))
    scale = 'F';
  else if (!strcmp(command, "SCALE=C\n"))
    scale = 'C';
  else if (is_prefix("PERIOD=", command))
    period = atoi(command + 7);
  else if (!strcmp(command, "STOP\n"))
    stopped = 1;
  else if (!strcmp(command, "START\n"))
    stopped = 0;
  else if (is_prefix("LOG\n", command))
    ;
  else if (!strcmp(command, "OFF\n"))
    running = 0;

  if (log_file) {
    fputs(command, log_file);
    fflush(log_file);
  }
}

double get_temperature() {
  // mraa_aio_read(temp_sens) = 999999
  double val = ((1023.0 * 100000.0) / (double)(999999)) - 1.0;
  double temp = 1.0 / (log((val / 100000.0)) / 4275 + 1 / 298.15) - 273.15;
  return scale == 'C' ? temp : temp * 9 / 5 + 32;
}

void print(char* s) {
  fputs(s, stdout);
  fflush(stdout);
  if (log_file) {
    fputs(s, log_file);
    fflush(log_file);
  }
}

int main(int argc, char** argv) {
  static const struct option long_options[] = {
      {"period", required_argument, NULL, PERIOD},
      {"scale", required_argument, NULL, SCALE},
      {"log", required_argument, NULL, LOG},
      {0, 0, 0, 0}};

  int ret;
  while ((ret = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
    switch (ret) {
      case PERIOD: {
        period = atoi(optarg);
        break;
      }
      case SCALE: {
        if (strlen(optarg) != 1 || (optarg[0] != 'F' && optarg[0] != 'C'))
          err("Invalid scale parameter: must be 'F' or 'C'");
        scale = optarg[0];
        break;
      }
      case LOG: {
        log_file = fopen(optarg, "w");
        if (!log_file) err("Invalid logfile");
        break;
      }
      case '?': {
        err("Invalid command line arugment given");
        break;
      }
    }
  }

  temp_sens = mraa_aio_init(1);
  but = mraa_gpio_init(62);
  mraa_gpio_dir(but, MRAA_GPIO_IN);

  struct pollfd pollfds[1];
  pollfds[0].fd = STDIN_FILENO;
  pollfds[0].events = POLLIN | POLLHUP | POLLERR;

  struct timeval time;
  int old_sec = -1;
  char time_buf[32];
  char out_buf[64];

  while (running) {
    gettimeofday(&time, 0);
    if (!stopped && (time.tv_sec - old_sec) >= period) {
      double temperature = get_temperature();
      strftime(time_buf, 32, "%H:%M:%S", localtime(&time.tv_sec));
      sprintf(out_buf, "%s %.1f\n", time_buf, temperature);
      print(out_buf);
      old_sec = time.tv_sec;
    }

    int retval = poll(pollfds, 1, 0);
    if (retval < 0) err("polling failure");

    if (pollfds[0].revents & POLLIN) {
      char command[1024];
      fgets(command, 1024, stdin);
      read_command(command);
    }

    // mraa_gpio_read(but) == 0
    if (0) {
      running = 0;
    }
  }

  gettimeofday(&time, 0);
  strftime(time_buf, 32, "%H:%M:%S", localtime(&time.tv_sec));
  sprintf(out_buf, "%s SHUTDOWN\n", time_buf);
  print(out_buf);

  mraa_aio_close(temp_sens);
  mraa_gpio_close(but);
  exit(0);
}