#ifndef HT_HASHTABLE
#define HT_HASHTABLE

typedef struct HashTableItem HashTableItem;
struct HashTableItem {
  char *key;
  char *value;
  HashTableItem *next;
};

typedef struct {
  int size;
  int base_size;
  int item_count;
  int list_count;
  HashTableItem **items;
} HashTable;

HashTable *ht_new();
void ht_insert(HashTable *ht, const char *key, const char *value);
char *ht_search(HashTable *ht, const char *key);
void ht_delete(HashTable *ht);
void ht_delete_item(HashTable *ht, const char *key);

// #define HT_IMPLEMENTATION
#ifdef HT_IMPLEMENTATION

#include <math.h>
#include <stdlib.h>
#include <string.h>

#define HT_PRIME_1 101
#define HT_PRIME_2 37
#define HT_INITIAL_BASE_SIZE 50

/*--------------------------INTERNAL UTILITY FUNCTIONS------------------------*/

static int is_prime(const int x);
static int next_prime(int x);

static HashTableItem *ht_new_item(const char *key, const char *value);
static HashTable *ht_new_sized(const int base_size);

static void ht_resize(HashTable *ht, const int base_size);
static void ht_resize_up(HashTable *ht);
static void ht_resize_down(HashTable *ht);

static void ht_free_item(HashTableItem *item);

static int ht_hash(const char *s, const int prime_num,
                   const int num_of_buckets);

/*------------------API FUNCTION IMPLEMENTATION------------------------------*/
HashTable *ht_new() { return ht_new_sized(HT_INITIAL_BASE_SIZE); }

void ht_delete(HashTable *ht) {

  for (int i = 0; i < ht->size; i++) {
    HashTableItem *item = ht->items[i];

    while (item != NULL) {
      HashTableItem *next = item->next;
      ht_free_item(item);
      item = next;
    }
  }
  free(ht->items);
  free(ht);
}

void ht_insert(HashTable *ht, const char *key, const char *value) {
  const int load = ht->item_count * 100 / ht->size;
  if (load > 70) {
    ht_resize_up(ht);
  }

  HashTableItem *item = ht_new_item(key, value);
  int index = ht_hash(key, HT_PRIME_1, ht->size);
  HashTableItem *current_item = ht->items[index];

  if (current_item == NULL) {
    ht->items[index] = item;
    ht->list_count++;
  } else {
    HashTableItem *prev_item = ht->items[index];
    HashTableItem *p = ht->items[index]->next;
    while (p != NULL) {
      prev_item = p;
      p = p->next;
    }
    prev_item->next = item;
  }
  ht->item_count++;
}

char *ht_search(HashTable *ht, const char *key) {
  int index = ht_hash(key, HT_PRIME_1, ht->size);
  HashTableItem *item = ht->items[index];

  while (strcmp(item->key, key) != 0) {
    item = item->next;
    if (item == NULL) {
      return NULL;
    }
  }

  return item->value;
}

void ht_delete_item(HashTable *ht, const char *key) {
  const int load = ht->item_count * 100 / ht->size;
  if (load < 10) {
    ht_resize_down(ht);
  }

  int index = ht_hash(key, HT_PRIME_1, ht->size);
  HashTableItem *item = ht->items[index];

  if (item == NULL) {
    return;
  }

  HashTableItem *prev = NULL;
  while (item != NULL) {
    if (strcmp(item->key, key) == 0) {
      if (prev == NULL) {
        ht->items[index] = item->next;
        ht_free_item(item);
        break;
      }
      prev->next = item->next;
      ht_free_item(item);
      break;
    }
    prev = item;
    item = item->next;
  }
  if (ht->items[index] == NULL) {
    ht->list_count--;
  }
  ht->item_count--;
}

/*-------------------STATIC FUNCTIONS DEFINATION-----------------------------*/

static HashTableItem *ht_new_item(const char *key, const char *value) {
  HashTableItem *i = (HashTableItem *)malloc(sizeof(HashTableItem));
  i->key = strdup(key);
  i->value = strdup(value);
  i->next = NULL;
  return i;
}

static HashTable *ht_new_sized(const int base_size) {
  HashTable *ht = malloc(sizeof(HashTable));

  ht->item_count = 0;
  ht->list_count = 0;

  ht->base_size = base_size;
  ht->size = next_prime(ht->base_size);

  ht->items = calloc((size_t)ht->size, sizeof(HashTableItem *));
  return ht;
}

static void ht_resize(HashTable *ht, const int base_size) {
  if (base_size < HT_INITIAL_BASE_SIZE) {
    return;
  }
  HashTable *new_ht = ht_new_sized(base_size);

  for (int i = 0; i < ht->size; i++) {
    HashTableItem *item = ht->items[i];

    while (item != NULL) {
      ht_insert(new_ht, item->key, item->value);
      item = item->next;
    }
  }

  ht->base_size = new_ht->base_size;
  ht->item_count = new_ht->item_count;
  ht->list_count = new_ht->list_count;

  int old_size = ht->size;
  ht->size = new_ht->size;
  new_ht->size = old_size;

  HashTableItem **old_items = ht->items;
  ht->items = new_ht->items;
  new_ht->items = old_items;

  ht_delete(new_ht);
}

static void ht_resize_up(HashTable *ht) {
  const int new_size = ht->base_size * 2;
  ht_resize(ht, new_size);
}

static void ht_resize_down(HashTable *ht) {
  const int new_size = ht->base_size / 2;
  ht_resize(ht, new_size);
}

static void ht_free_item(HashTableItem *item) {
  free(item->key);
  free(item->value);
  free(item);
}

static int ht_hash(const char *s, const int prime_num,
                   const int num_of_buckets) {
  long hash = 0;
  const int len_s = strlen(s);
  for (int i = 0; i < len_s; i++) {
    hash += (long)pow(prime_num, len_s - (i + 1)) * s[i];
    hash = hash % num_of_buckets;
  }

  return (int)hash;
}

/*
 * Returns:
 *  1  - prime
 *  0  - not prime
 *  -1 - undefined (i.e. x < 2)
 */
static int is_prime(const int x) {
  if (x < 2) {
    return -1;
  }
  if (x < 4) {
    return 1;
  }
  if ((x % 2) == 0) {
    return 0;
  }
  for (int i = 3; i <= floor(sqrt((double)x)); i += 2) {
    if ((x % i) == 0) {
      return 0;
    }
  }
  return 1;
}

static int next_prime(int x) {
  while (is_prime(x) != 1) {
    x++;
  }
  return x;
}

#endif // HT_IMPLEMENTATION

#endif // !HT_HASHTABLE
