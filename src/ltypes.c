#include "ltypes.h"
#include "llog.h"
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

u32 hash(const char *str) {
  u32 base = 5381;
  u32 c;
  while ((c = *str++)) {
    base = ((base << 5) + base) + c;
  }

  return base;
};

u32 hash_len(const char *str, u32 len) {
  u32 base = 5381;
  u32 c;
  for (u32 i = 0; i < len; i++) {
    c = str[i];
    base = ((base << 5) + base) + c;
  }

  return base;
};

void *xmalloc(size_t len) {
  void *alloc = malloc(len);
  if (alloc == NULL) {
    ERROR_LOG("Failed to allocate memory\n");
    abort();
  }

  return alloc;
};

void xfree(void *ptr) {
  if (ptr == NULL)
    return;

  free(ptr);
};

void *xrealloc(void *ptr, size_t newsize) {
  void *alloc = realloc(ptr, newsize);
  if (alloc == NULL) {
    ERROR_LOG("Failed to reallocate memory\n");
    abort();
  }

  return alloc;
};

fbuffer_t new_fbuf(size_t init) {
  fbuffer_t buf;
  buf.cap = init;
  buf.length = 0;
  buf.buf = xmalloc(init);

  return buf;
};

void fbuf_write(fbuffer_t *buf, char *format, ...) {
  va_list args;
  va_start(args, format);
  int len = vsnprintf(NULL, 0, format, args);
  va_end(args);

  if (buf->length + len >= buf->cap) {
    buf->cap = buf->cap * 2;
    buf->buf = xrealloc(buf->buf, buf->cap);
  }

  va_start(args, format);
  vsnprintf(buf->buf + buf->length, len + 1, format, args);
  va_end(args);

  buf->length += len;
};

char *fbuf_get(fbuffer_t *buf) { return buf->buf; };
void fbuf_reset(fbuffer_t *buf) {
  buf->length = 0;
  buf->buf[0] = '\0';
}

void fbuf_free(fbuffer_t *buf) {
  CHECK_NULL(buf, );

  xfree(buf->buf);
};

list_t new_list(size_t init) {
  list_t l;
  l.cap = init;
  l.length = 0;
  l.items = xmalloc(init * sizeof(void *));
  return l;
};

void list_push(list_t *list, void *item) {
  if (list->length >= list->cap) {
    list->cap = list->cap * 2;
    list->items = xrealloc(list->items, list->cap * sizeof(void *));
  }

  list->items[list->length++] = item;
};

void list_free(list_t *list, void (*item_free)(void *)) {
  if (item_free != NULL) {
    for (size_t i = 0; i < list->length; i++) {
      item_free(list->items[i]);
    }
  }

  xfree(list->items);
};