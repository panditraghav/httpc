#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "usage: httpc hostname\n");
    return 1;
  }

  int gai_status;
  struct addrinfo *info;
  struct addrinfo hints = {0};

  hints.ai_family = AF_UNSPEC; // Don't care if IPv4 or IPv6
  // hints.ai_flags = AI_PASSIVE;     // Fill in my IP address
  hints.ai_socktype = SOCK_STREAM; // TCP

  if ((gai_status = getaddrinfo(argv[1], NULL, &hints, &info)) !=
      0) {
    fprintf(stderr, "Some error occured: %s\n", gai_strerror(gai_status));
    return 2;
  }

  for (struct addrinfo *p = info; p != NULL; p = p->ai_next) {
    char *family;
    char ipstr[INET6_ADDRSTRLEN];
    void *addr;

    if (p->ai_family == AF_INET) {
      // IPv4
      family = "IPv4";
      struct sockaddr_in *sin = (struct sockaddr_in *)p->ai_addr;
      addr = &(sin->sin_addr);
    } else {
      struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(sin6->sin6_addr);
      family = "IPv6";
    }

    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
    printf("%s : %s\n", family, ipstr);
  }

  freeaddrinfo(info);
}
