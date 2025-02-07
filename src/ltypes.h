#pragma once

#include <stddef.h>
#include <stdint.h>

typedef uint32_t u32;
typedef int64_t s64;
typedef double f64;

u32 hash(const char *str);
u32 hash_len(const char *str, u32 len);

typedef struct {
  char *buf;
  size_t length;
  size_t cap;
} fbuffer_t;

void *xmalloc(size_t len);
void xfree(void *ptr);
void *xrealloc(void *ptr, size_t newsize);

fbuffer_t new_fbuf(size_t init);
void fbuf_write(fbuffer_t *buf, char *format, ...);
char *fbuf_get(fbuffer_t *buf);
void fbuf_reset(fbuffer_t *buf);
void fbuf_free(fbuffer_t *buf);

typedef struct {
  void **items;
  size_t length;
  size_t cap;
} list_t;

list_t new_list(size_t init);
void list_push(list_t *list, void *item);
void list_free(list_t *list, void (*item_free)(void *));