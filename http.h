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

void parse_http_request_head(char *request, size_t req_size);
void sigchild_handler(int s);
void serve_request();

// TODO: Remove this line
#define HTTP_IMPLEMENTATION

#ifdef HTTP_IMPLEMENTATION

#define BACKLOG 20
#define RECV_BUFF_SIZE 512

void sigchild_handler(int s) {
  (void)s; // Quite unused variable warnings
  int saved_errno = errno;

  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;

  errno = saved_errno;
}

void parse_http_request_head(char *request, size_t req_size) {
  size_t req_i = 0;
  char line[512];
  size_t line_i = 0;
  size_t line_read = 0;
  char http_method[7], resource_url[100], http_version[8];

  while (req_i < req_size) {

    while (strncmp(request + req_i, "\r\n", 2) != 0) {
      line[line_i++] = request[req_i++];
      if (line_i == 200) {
        line[line_i] = '\0';
        puts(line);
        return;
      }
    }

    line[line_i] = '\0';
    req_i += 2;
    line_i = 0;

    line_read++;
    if (line_read == 1) {
      sscanf(line, "%s %s %s", http_method, resource_url, http_version);
      printf("Method: %s\nResource URL: %s\nVersion: %s\n", http_method,
             resource_url, http_version);
      continue;
    }

    char key[50] = {0}, value[100] = {0};
    // Example: Content-Length: 44
    sscanf(line, "%s %100c", key, value);
    key[strlen(key) - 1] = '\0'; // Removing :
    printf("%s: %s\n", key, value);
  }
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
      int recvcount = 0;
      char recv_buff[RECV_BUFF_SIZE] = {0};
      char req_head[RECV_BUFF_SIZE] = {0};
      size_t req_head_i = 0;
      char got_head = 0;

      while (1) {
        if ((recvcount = recv(new_fd, recv_buff, RECV_BUFF_SIZE, 0)) < 0) {
          fprintf(stderr, "[ERROR] recv: %s\n", strerror(errno));
          break;
        }
        printf("Received message of size %d bytes:\n", recvcount);

        size_t recv_buff_i = 0;
        while (!got_head) {
          if (strncmp(recv_buff + recv_buff_i, "\r\n\r\n", 4) == 0) {
            got_head = 1;
            strcpy(req_head + req_head_i, "\r\n");
            req_head[req_head_i + 2] = '\0';
            break;
          }
          req_head[req_head_i++] = recv_buff[recv_buff_i++];
        }

        if (got_head) {
          parse_http_request_head(req_head, req_head_i);
          // TODO: Put rest of the recv_buff data after \r\n\r\n into a new
          // req_body buffer
        }

        if (recvcount < RECV_BUFF_SIZE) {
          break;
        }
      }
    }
    close(new_fd); // Parent doesn't need this
  }
}
#endif // HTTP_IMPLEMENTATION
