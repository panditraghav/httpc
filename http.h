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

void parse_http_request(char *request, size_t req_size);
void sigchild_handler(int s);
void serve_request();

// TODO: Remove this line
#define HTTP_IMPLEMENTATION

#ifdef HTTP_IMPLEMENTATION

#define BACKLOG 20

void sigchild_handler(int s) {
  (void)s; // Quite unused variable warnings
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

void parse_http_request(char *request, size_t req_size) {
  char line[200];
  size_t line_i = 0;
  size_t req_i = 0;

  while (req_i < req_size) {
    while (!(request[req_i] == '\r' && request[req_i + 1] == '\n')) {
      line[line_i++] = request[req_i++];
      if (line_i == 200) {
        line[line_i] = '\0';
        printf("greater: ");
        puts(line);
        return;
      }
    }
    line[line_i] = '\0';
    printf("line: ");
    puts(line);
    req_i += 2;
    line_i = 0;
  }
  printf("req_size: %zu, req_i: %zu, line_i: %zu\n", req_size, req_i, line_i);
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
      fprintf(stderr, "[ERROR] accept\n");
      continue;
    }

    if (!fork()) {
      close(sockfd); // child doesn't need the listener
      if (send(new_fd, "Hello, world!", 13, 0) == -1) {
        fprintf(stderr, "[ERROR] send\n");
        close(new_fd);
        exit(EXIT_SUCCESS);
      }
    }
    close(new_fd); // Parent doesn't need this
  }
}
#endif // HTTP_IMPLEMENTATION
