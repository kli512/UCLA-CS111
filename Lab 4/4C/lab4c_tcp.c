// NAME: Kevin Li
// EMAIL: li.kevin512@gmail.com
// ID: 123456789
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

// getopt options
#define SCALE 1
#define PERIOD 2
#define LOG 3
#define ID 4
#define HOST 5

// program globals
int period = 1;
char scale = 'F';
FILE* log_file = NULL;
int running = 1;
int stopped = 0;
char* id = NULL;

// server setup
int sock_fd;

// mraa setup
mraa_aio_context temp_sens;
mraa_gpio_context but;

// error with nonzero exit code
void err(char* error) {
  fprintf(stderr, "%s\n", error);
  exit(1);
}

// checks if pre is a prefix of s
int is_prefix(char* pre, char* s) {
  if (strlen(pre) > strlen(s)) return 0;
  return !memcmp(pre, s, strlen(pre));
}

// parses and executes commands, and logs them
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

// grabs and converts temperature measurements
double get_temperature() {
  double val = ((1023.0 * 100000.0) / (double)(mraa_aio_read(temp_sens))) - 1.0;
  double temp = 1.0 / (log((val / 100000.0)) / 4275 + 1 / 298.15) - 273.15;
  return scale == 'C' ? temp : temp * 9 / 5 + 32;
}

// prints to both stdout and log if necessary
void print(char* s) {
  dprintf(sock_fd, "%s", s);
  if (log_file) {
    fputs(s, log_file);
    fflush(log_file);
  }
}

int main(int argc, char** argv) {
  char* host;
  // option processing - valid opts include

  // period - chooses the period between logs
  // scale - C or F for celsius or fahrenheit
  // log - log file name
  static const struct option long_options[] = {
      {"period", optional_argument, NULL, PERIOD},
      {"scale", optional_argument, NULL, SCALE},
      {"id", required_argument, NULL, ID},
      {"host", required_argument, NULL, HOST},
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
      case ID: {
        id = optarg;
        break;
      }
      case HOST: {
        host = optarg;
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

  // open socket
  sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) err("Failed to open socket");

  // find server
  struct hostent* server = gethostbyname(host);
  if (server == NULL) err("Invalid host server");

  // connect to server
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  memcpy((char *)&server_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
  server_addr.sin_port = htons(atoi(argv[argc - 1])); // port should be pushed to end of argv
  if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    err("Error connecting to host");

  print("ID=");
  print(id);

  // sensor setup
  temp_sens = mraa_aio_init(1);
  but = mraa_gpio_init(60);
  mraa_gpio_dir(but, MRAA_GPIO_IN);

  // stdin polling setup
  struct pollfd pollfds[1];
  pollfds[0].fd = sock_fd;
  pollfds[0].events = POLLIN | POLLHUP | POLLERR;

  // text output setup
  struct timeval time;
  int old_sec = -1;
  char time_buf[32];
  char out_buf[64];

  // runs until something kills it i.e. button or OFF
  while (running) {
    // gets and prints time and temp if enough time passed
    gettimeofday(&time, 0);
    if (!stopped && (time.tv_sec - old_sec) >= period) {
      double temperature = get_temperature();
      strftime(time_buf, 32, "%H:%M:%S", localtime(&time.tv_sec));
      sprintf(out_buf, "%s %.1f\n", time_buf, temperature);
      print(out_buf);
      old_sec = time.tv_sec;
    }

    // tries to poll STDIN with error catching
    if (poll(pollfds, 1, 0) < 0) err("polling failure");

    // reads stdin and pushes command to read_command() function
    if (pollfds[0].revents & POLLIN) {
			char command[1024];
			fgets(command, 1024, fdopen(sock_fd, "r"));
      read_command(command);
    }

    // checks if button pressed
    if (mraa_gpio_read(but)) {
      running = 0;
    }
  }

  // output with SHUTDOWN as final log
  gettimeofday(&time, 0);
  strftime(time_buf, 32, "%H:%M:%S", localtime(&time.tv_sec));
  sprintf(out_buf, "%s SHUTDOWN\n", time_buf);
  print(out_buf);

  // exiting with cleanup
  mraa_aio_close(temp_sens);
  mraa_gpio_close(but);
  exit(0);
}