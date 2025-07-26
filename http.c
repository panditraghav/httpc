#define HT_IMPLEMENTATION
#include "hash_table.h"

#include "http.h"
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>

#define BACKLOG 20
#define RECV_BUFF_SIZE 512 * 1024
// 10MB max
#define MAX_RECV_BUFF_SIZE 10 * 1024 * 1024

void sigchild_handler(int s) {
  (void)s; // Quite unused variable warnings
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

/*
 * Parse head from request request, and returns index from where body starts
 */
int parse_head_from_request(char *request, size_t req_size, RequestHead *head) {
  size_t number_of_line_read = 0;
  size_t line_start = 0;
  size_t line_end = 0;

  while (line_end < req_size) {
    while (strncmp(request + line_end, "\r\n", 2) != 0) {
      line_end++;
    }

    size_t line_size = line_end - line_start;
    char line[line_size];
    strncpy(line, request + line_start, line_size);
    line[line_size] = '\0';
    number_of_line_read++;

    // Get first line: POST / HTTP/1.1
    if (number_of_line_read == 1) {
      sscanf(line, "%s %s %s", head->http_method, head->resource_url,
             head->http_version);
    } else {
      char key[50] = {0}, value[100] = {0};
      // Example: Content-Length: 44
      sscanf(line, "%s %100c", key, value);
      key[strlen(key) - 1] = '\0'; // Removing ':'

      ht_insert(head->headers, key, value);
    }

    if (strncmp(request + line_end, "\r\n\r\n", 4) == 0) {
      line_start = line_end + 4;
      break;
    }

    // Current line_end is at '\r', next line_start will be after '\r\n'
    line_end += 2;
    line_start = line_end;
  }

  return line_start;
}

void create_server(char *port) {
  int sockfd, new_fd;
  struct addrinfo hints = {0}, *servinfo, *p;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  int rv;
  if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "[ERROR] getaddrinfo: %s\n", gai_strerror(rv));
    exit(EXIT_FAILURE);
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      fprintf(stderr, "[ERROR] socket\n");
      continue;
    }

    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) < 0) {
      fprintf(stderr, "[ERROR] setsockopt\n");
      continue;
    }

    if (bind(sockfd, p->ai_addr, p->ai_addrlen) < 0) {
      close(sockfd);
      fprintf(stderr, "[ERROR] bind\n");
      continue;
    }
    break;
  }
  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "[ERROR] Failed to bind\n");
    exit(EXIT_FAILURE);
  }

  if (listen(sockfd, BACKLOG) < 0) {
    fprintf(stderr, "[ERROR] Failed to bind\n");
    exit(EXIT_FAILURE);
  }

  struct sigaction sa;
  sa.sa_handler = sigchild_handler; // Reap all dead process
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  if (sigaction(SIGCHLD, &sa, NULL) < 0) {
    fprintf(stderr, "[ERROR] Sigaction\n");
    exit(EXIT_FAILURE);
  }

  printf("[INFO] Server is listening at PORT %s\n", port);

  struct sockaddr_storage their_addr; // connector's address info
  while (1) {
    socklen_t sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

    if (new_fd < 0) {
      fprintf(stderr, "[ERROR] accept: %s\n", strerror(errno));
      continue;
    }

    if (!fork()) {
      size_t recv_buff_size = RECV_BUFF_SIZE;
      size_t total_bytes_received = 0;
      char *recv_buff = (char *)malloc(recv_buff_size);

      while (1) {
        ssize_t recv_count = recv(new_fd, recv_buff + total_bytes_received,
                                  recv_buff_size - total_bytes_received - 1, 0);

        if (recv_count < 0) {
          fprintf(stderr, "[ERROR] recv: %s\n", strerror(errno));
          _exit(EXIT_FAILURE);
          break;
        }

        total_bytes_received += recv_count;

        if (recv_count < recv_buff_size - 1) {
          recv_buff[total_bytes_received + recv_count] = '\0';
          break;
        }
        if (recv_count == recv_buff_size - 1) {

          if (recv_buff_size == MAX_RECV_BUFF_SIZE) {
            fprintf(stderr, "Maxed out recv_buff_size!\n");
            recv_buff[total_bytes_received + recv_count] = '\0';
            break;
          }
          printf("[INFO] content length exceeded %zu, doubling buffer size\n",
                 recv_buff_size);

          char *new_recv_buff = (char *)malloc(recv_buff_size * 2);
          memcpy(new_recv_buff, recv_buff, recv_buff_size);
          free(recv_buff);
          recv_buff = new_recv_buff;
          recv_buff_size *= 2;
        }
      }
      printf("[INFO] received content of length: %zu\n", total_bytes_received);
      puts(recv_buff);

      size_t req_header_size = 0;

      while (strncmp(recv_buff + req_header_size, "\r\n\r\n", 4) != 0) {
        req_header_size++;
      }

      RequestHead head = {{0}, .headers = ht_new()};

      parse_head_from_request(recv_buff, req_header_size, &head);

      const char *send_buf =
          "HTTP/1.1 200 OK\r\n\r\nThis is the response body!";

      ssize_t send_count = send(new_fd, send_buf, strlen(send_buf), 0);

      if (send_count < 0) {
        fprintf(stderr, "[ERROR] send: %s\n", strerror(errno));
        _exit(EXIT_FAILURE);
        break;
      }
      printf("[INFO] sent %zu bytes\n", send_count);

      _exit(EXIT_SUCCESS);
    }
    close(new_fd); // Parent doesn't need this
  }
}
