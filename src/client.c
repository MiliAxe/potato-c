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

int connect_socket(int port)
{
  int status, valread, client_fd;
  struct sockaddr_in serv_addr;
  char buffer[1024] = { 0 };
  if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("\n Socket creation error \n");
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

int send_socket_request_return_num(SocketRequest req, int pid)
{
  char buffer[1024];
  size_t size = int_length(req)+1;
  char* request = malloc(size*sizeof(char));
  snprintf(request, size, "%d",req);
  
  int sockfd = connect_socket(return_sock_port_from_number(pid));
  if (sockfd == -1)
    return -1;

  send(sockfd, request, size, 0);

  int valread = read(sockfd, buffer, 1024 - 1);
  close(sockfd);
  int output;
  sscanf(buffer, "%d", &output);
  return output;
}
