#include "hash_table.h"
#include <stdlib.h>

typedef struct {
  char http_method[7];
  char resource_url[100];
  char http_version[8];
  HashTable *headers;
} RequestHead;

int parse_head_from_request(char *request, size_t req_size, RequestHead *head);
void sigchild_handler(int s);
void serve_request();
void create_server(char *port);
