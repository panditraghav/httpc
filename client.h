#ifdef TEST
void client() {
  int gai_status;
  struct addrinfo *info;
  struct addrinfo hints = {0};
  struct addrinfo *p = NULL;
  int sockfd = 0;

  hints.ai_family = AF_UNSPEC;     // Don't care if IPv4 or IPv6
  hints.ai_flags = AI_PASSIVE;     // Fill in my IP address
  hints.ai_socktype = SOCK_STREAM; // TCP

  if ((gai_status = getaddrinfo("www.example.com", "80", &hints, &info)) != 0) {
    fprintf(stderr, "[ERROR] getaddrinfo: %s\n", gai_strerror(gai_status));
    return 2;
  }

  for (p = info; p != NULL; p = p->ai_next) {
    int connstatus = 0;

    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0) {
      fprintf(stderr, "[ERROR] socket: %s\n", strerror(errno));
      continue;
    }

    if ((connstatus = connect(sockfd, p->ai_addr, p->ai_addrlen)) != 0) {
      close(sockfd);
      fprintf(stderr, "[ERROR] connect: %s\n", strerror(errno));
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "[ERROR] unable to make connection to any address!\n");
    return 1;
  }

  char destination_address[INET6_ADDRSTRLEN];
  destination_address[INET6_ADDRSTRLEN - 1] = '\0';

  if (p->ai_family == AF_INET) {
    struct sockaddr_in *sin = (struct sockaddr_in *)p->ai_addr;
    inet_ntop(p->ai_family, &sin->sin_addr, destination_address,
              sizeof destination_address);
  } else {
    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)p->ai_addr;
    inet_ntop(p->ai_family, &sin6->sin6_addr, destination_address,
              sizeof destination_address);
  }

  printf("[INFO] connected to address: %s\n", destination_address);

  int sendcount = 0, recvcount = 0;
  char *send_buff = "GET / HTTP/1.1\r\nHost: www.example.com\r\n\r\n";
  size_t recv_buff_size = 512;
  char recv_buff[recv_buff_size];

  if ((sendcount = send(sockfd, send_buff, strlen(send_buff), 0)) < 0) {
    fprintf(stderr, "[ERROR] send: %s\n", strerror(errno));
    return 1;
  }

  printf("[INFO] sent %d bytes\n", sendcount);
  printf("[INFO] Message receved:\n");

  while (1) {
    if ((recvcount = recv(sockfd, recv_buff, recv_buff_size, 0)) < 0) {
      fprintf(stderr, "[ERROR] recv: %s\n", strerror(errno));
      return 1;
    }
    recv_buff[recvcount] = '\0';
    parse_http_request(recv_buff, recvcount);

    if (recvcount < recv_buff_size) {
      break;
    }
  }

  close(sockfd);
  freeaddrinfo(info);
  return 0;
}
#endif // TEST
