#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

#include "../include/signal.h"
#include "../include/client.h"
#include "../include/socket.h"
#include "../include/utils.h"
#include "../include/timer.h"
#include "../include/pidfile.h"

#define handle_kill(NAME, SIG) void handle_##NAME(char *str, int index)\
{\
  kill(atoi(str), SIG);\
}

handle_kill(pause, SIG_PAUSE);
handle_kill(quit, SIGQUIT);
handle_kill(unpause, SIG_UNPAUSE);
handle_kill(skip, SIG_SKIP);
handle_kill(toggle_pause, SIG_TPAUSE);
handle_kill(increase_10sec, SIG_INC_10SEC);
handle_kill(decrease_10sec, SIG_DEC_10SEC);
handle_kill(increase_pomodoro_count, SIG_INC_POMODORO_COUNT);
handle_kill(decrease_pomodoro_count, SIG_DEC_POMODORO_COUNT);
handle_kill(reset_pomodoro, SIG_RESET);

void run_function_on_pid_file_index(void(* handler)(char *, int index), int selected_index)
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir (POTATO_PIDS_DIRECTORY);
  int index = 0;

  if (dp != NULL)
  {
    while ((ep = readdir (dp)) != NULL) {
      if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
        if (index == selected_index || selected_index == EVERY_MEMBER)
          handler(ep->d_name, index);
        index++;
      }
    }

    (void) closedir (dp);
    return;
  }
}

void handle_remove_pid(char *name, int index)
{
  remove_pid_file(atoi(name));
}

void run_function_on_pid_file_pid(void(* handler)(char *, int index), int selected_pid)
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir (POTATO_PIDS_DIRECTORY);
  int index = 0;

  if (dp != NULL)
  {
    while ((ep = readdir (dp)) != NULL) {
      if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
        if (atoi(ep->d_name) == selected_pid || selected_pid == EVERY_MEMBER)
          handler(ep->d_name, index);
        index++;
      }
    }

    (void) closedir (dp);
    return;
  }
}

unsigned int get_pids_length()
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir(POTATO_PIDS_DIRECTORY);

  unsigned int output = 0;
  if (dp != NULL)
  {
    while ((ep = readdir (dp)) != NULL) {
      if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
        output++;
      }
    }
  }
  free(dp);
  return output;
}

Timer * request_timer(pid_t pid)
{
  Timer *timer = malloc(sizeof(Timer));
  timer->seconds = send_socket_request_return_num(REQ_SECONDS,pid);
  if (timer->seconds == -1) {
    free(timer);
    return NULL;
  }
  timer->type = send_socket_request_return_num(REQ_TYPE,pid);
  timer->pomodoro_count = send_socket_request_return_num(REQ_POMODOROS,pid);
  timer->paused = send_socket_request_return_num(REQ_PAUSED, pid);
  return timer;
}

pid_t pid_at_index(unsigned int selected_index)
{
  DIR *dp;
  struct dirent *ep;
  dp = opendir (POTATO_PIDS_DIRECTORY);

  pid_t output = 0;
  int index =0;
  if (dp != NULL)
  {
    while ((ep = readdir (dp)) != NULL) {
      if (strcmp(ep->d_name, ".") && strcmp(ep->d_name, "..")) {
        if (index == selected_index)
          output = atoi(ep->d_name);
        index++;
      }
    }
  }
  free(dp);
  return output;
}

int is_socket_available(int port) 
{
  int sockfd;
  struct sockaddr_in serverAddr;

  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Set server address details
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);
  serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Replace with desired IP address

  // Connect to the server
  int connectionStatus = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
  if (connectionStatus == 0) {
    close(sockfd);
    return 1; // Socket connection exists
  }

  close(sockfd);
  return 0; // Socket connection does not exist
}

int connect_socket(int port)
{
  int status, valread, client_fd;
  struct sockaddr_in serv_addr;
  if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  // Convert IPv4 and IPv6 addresses from text to binary
  // form
  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    printf(
      "\nInvalid address/ Address not supported \n");
    return -1;
  }

  if ((status = connect(client_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) < 0) {
    return -1;
  }

  return client_fd;
}

char * send_req_return_str(SocketRequest req, int sockfd)
{
  char *buffer = malloc(sizeof(char)*1024);
  size_t size = int_length(req)+1;
  char* request = malloc(size*sizeof(char));
  snprintf(request, size, "%d",req);

  send(sockfd, request, size, 0);
  free(request);
  int valread = read(sockfd, buffer, 1024 - 1);
  close(sockfd);

  return buffer;
}

int send_socket_request_with_fd(SocketRequest req, int sockfd)
{
  char * buffer = send_req_return_str(req, sockfd);

  int output;
  sscanf(buffer, "%d", &output);
  free(buffer);
  return output;
}

int send_socket_request_return_num(SocketRequest req, int pid)
{
  int port = read_sock_port_from_pid_file(pid);
  // printf("%d\n",port);
  int sockfd = connect_socket(port);
  if (sockfd == NO_PORT)
    return NO_PORT;

  int output = send_socket_request_with_fd(req, sockfd);
  return output;
}

Timer get_timer_pid(pid_t pid)
{
  int port = read_sock_port_from_pid_file(pid);
  // printf("%d\n",port);
  int sockfd = connect_socket(port);
  char * buffer = send_req_return_str(REQ_TIMER_FULL, sockfd);

  Timer timer;
  int scanned = sscanf(buffer, "%d-%d-%d-%d", &timer.seconds, &timer.pomodoro_count, &timer.paused, &timer.type);
  free(buffer);

  if (scanned < 4) {
    timer.type = -1;
  }
  return timer;
}
