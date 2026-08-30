#include "strbuffer.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static unsigned char hn_empty_buf[1];
static int hn_is_space(unsigned char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

void strbuffer_init(struct strbuffer *buf, size_t capacity)
{
  if (!buf) return;
  buf->data = hn_empty_buf; buf->len = 0; buf->cap = 0; buf->failed = 0;
  if (capacity) (void)strbuffer_reserve(buf, capacity);
}

void strbuffer_release(struct strbuffer *buf)
{
  if (!buf) return;
  if (buf->data != hn_empty_buf) free(buf->data);
  strbuffer_init(buf, 0);
}

void strbuffer_reset(struct strbuffer *buf)
{
  if (!buf) return;
  buf->len = 0; buf->data[0] = '\0';
}

int strbuffer_reserve(struct strbuffer *buf, size_t capacity)
{
  unsigned char *new_data;
  size_t new_cap;
  if (!buf) return 0;
  if (buf->failed) return 0;
  if (capacity == SIZE_MAX) { buf->failed = 1; return 0; }
  if (capacity + 1 <= buf->cap) return 1;
  new_cap = buf->cap ? buf->cap : 64;
  while (new_cap <= capacity) {
    size_t grown = new_cap + (new_cap >> 1);
    if (grown <= new_cap || grown > SIZE_MAX - 1) {
      new_cap = capacity + 1; break;
    }
    new_cap = grown;
  }
  new_data = realloc(buf->data == hn_empty_buf ? NULL : buf->data, new_cap);
  if (!new_data) { buf->failed = 1; return 0; }
  buf->data = new_data; buf->cap = new_cap; buf->data[buf->len] = '\0';
  return 1;
}

static int strbuffer_make_room(struct strbuffer *buf, size_t extra)
{
  if (!buf || buf->failed) return 0;
  if (extra > SIZE_MAX - buf->len) { buf->failed = 1; return 0; }
  return strbuffer_reserve(buf, buf->len + extra);
}

int strbuffer_set(struct strbuffer *buf, const unsigned char *data, size_t len)
{
  if (!buf) return 0;
  if (!data && len) { buf->failed = 1; return 0; }
  if (!len) { strbuffer_reset(buf); return 1; }
  if (!strbuffer_reserve(buf, len)) return 0;
  memmove(buf->data, data, len); buf->len = len; buf->data[len] = '\0';
  return 1;
}

int strbuffer_set_str(struct strbuffer *buf, const char *str)
{
  if (!buf) return 0;
  if (!str) { buf->failed = 1; return 0; }
  return strbuffer_set(buf, (const unsigned char *)str, strlen(str));
}

int strbuffer_add_byte(struct strbuffer *buf, unsigned char byte)

{
  if (!strbuffer_make_room(buf, 1)) return 0;
  buf->data[buf->len++] = byte; buf->data[buf->len] = '\0'; return 1;
}

int strbuffer_add(struct strbuffer *buf, const unsigned char *data, size_t len)
{
  size_t old_len;
  size_t data_offset = 0;
  int data_is_internal;
  if (!buf) return 0;
  if (!data && len) { buf->failed = 1; return 0; }
  if (!len) return 1;
  old_len = buf->len;
  data_is_internal = (uintptr_t)data >= (uintptr_t)buf->data &&
                     (uintptr_t)data <= (uintptr_t)buf->data + buf->len;
  if (data_is_internal) {
    data_offset = (size_t)(data - buf->data);
    if (len > buf->len + 1 - data_offset) return 0;
  }
  if (!strbuffer_make_room(buf, len)) return 0;
  if (data_is_internal) data = buf->data + data_offset;
  memmove(buf->data + old_len, data, len);
  buf->len = old_len + len; buf->data[buf->len] = '\0'; return 1;
}

int strbuffer_add_str(struct strbuffer *buf, const char *str)
{
  if (!buf) return 0;
  if (!str) { buf->failed = 1; return 0; }
  return strbuffer_add(buf, (const unsigned char *)str, strlen(str));
}

int strbuffer_add_utf8(struct strbuffer *buf, unsigned int cp)
{
  unsigned char bytes[4]; size_t len;
  if (cp <= 0x7f) { bytes[0] = (unsigned char)cp; len = 1; }
  else if (cp <= 0x7ff) {
    bytes[0] = (unsigned char)(0xc0 | (cp >> 6));
    bytes[1] = (unsigned char)(0x80 | (cp & 0x3f)); len = 2;
  } else if (cp <= 0xffff && !(cp >= 0xd800 && cp <= 0xdfff)) {
    bytes[0] = (unsigned char)(0xe0 | (cp >> 12));
    bytes[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3f));
    bytes[2] = (unsigned char)(0x80 | (cp & 0x3f)); len = 3;
  } else if (cp <= 0x10ffff) {
    bytes[0] = (unsigned char)(0xf0 | (cp >> 18));
    bytes[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3f));
    bytes[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3f));
    bytes[3] = (unsigned char)(0x80 | (cp & 0x3f)); len = 4;
  } else {
    if (buf) buf->failed = 1;
    return 0;
  }
  return strbuffer_add(buf, bytes, len);
}

size_t strbuffer_len(const struct strbuffer *buf) { return buf ? buf->len : 0; }

void strbuffer_copy(char *dst, size_t dst_size, const struct strbuffer *src)
{
  size_t n;
  if (!dst || !dst_size) return;
  if (!src) { dst[0] = '\0'; return; }
  n = src->len < dst_size - 1 ? src->len : dst_size - 1;
  memmove(dst, src->data, n); dst[n] = '\0';
}

void strbuffer_swap(struct strbuffer *a, struct strbuffer *b)
{ struct strbuffer tmp = *a; *a = *b; *b = tmp; }

unsigned char *strbuffer_steal(struct strbuffer *buf)
{
  unsigned char *data;
  if (!buf) return NULL;
  if (buf->failed) { strbuffer_release(buf); return NULL; }
  if (buf->data == hn_empty_buf) {
    data = calloc(1, 1);
    strbuffer_init(buf, 0);
    return data;
  }
  data = buf->data; strbuffer_init(buf, 0); return data;
}

int strbuffer_cmp(const struct strbuffer *a, const struct strbuffer *b)
{
  size_t n = a->len < b->len ? a->len : b->len;
  int cmp = memcmp(a->data, b->data, n);
  return cmp ? cmp : (a->len > b->len) - (a->len < b->len);
}

ptrdiff_t strbuffer_find(const struct strbuffer *buf, int byte, size_t start)
{
  const unsigned char *found;
  if (!buf || start >= buf->len) return -1;
  found = memchr(buf->data + start, byte, buf->len - start);
  return found ? found - buf->data : -1;
}

ptrdiff_t strbuffer_rfind(const struct strbuffer *buf, int byte, size_t start)
{
  size_t pos;
  if (!buf || !buf->len) return -1;
  pos = start < buf->len ? start + 1 : buf->len;
  while (pos) if (buf->data[--pos] == (unsigned char)byte) return (ptrdiff_t)pos;
  return -1;
}

void strbuffer_truncate(struct strbuffer *buf, size_t len)
{ if (buf && len < buf->len) { buf->len = len; buf->data[len] = '\0'; } }

void strbuffer_consume(struct strbuffer *buf, size_t len)
{
  if (!buf || !len) return;
  if (len >= buf->len) { strbuffer_reset(buf); return; }
  memmove(buf->data, buf->data + len, buf->len - len);
  buf->len -= len; buf->data[buf->len] = '\0';
}

void strbuffer_trim_right(struct strbuffer *buf)
{
  if (!buf) return;
  while (buf->len && hn_is_space(buf->data[buf->len - 1])) --buf->len;
  buf->data[buf->len] = '\0';
}

void strbuffer_trim(struct strbuffer *buf)
{
  size_t start = 0;
  if (!buf) return;
  while (start < buf->len && hn_is_space(buf->data[start])) ++start;
  strbuffer_consume(buf, start); strbuffer_trim_right(buf);
}

void strbuffer_squash_spaces(struct strbuffer *buf)
{
  size_t r, w = 0; int pending = 0;
  if (!buf) return;
  for (r = 0; r < buf->len; ++r) {
    if (hn_is_space(buf->data[r])) { pending = w != 0; continue; }
    if (pending) buf->data[w++] = ' ';
    buf->data[w++] = buf->data[r]; pending = 0;
  }
  buf->len = w; buf->data[w] = '\0';
}

void strbuffer_unescape(struct strbuffer *buf)
{
  size_t r, w = 0;
  if (!buf) return;
  for (r = 0; r < buf->len; ++r) {
    if (buf->data[r] == '\\' && r + 1 < buf->len && ispunct(buf->data[r + 1])) ++r;
    buf->data[w++] = buf->data[r];
  }
  buf->len = w; buf->data[w] = '\0';
}

char *hn_strdup(const char *str) { return str ? hn_strndup(str, strlen(str)) : NULL; }

char *hn_strndup(const char *str, size_t len)
{
  char *copy;
  if (!str || len == SIZE_MAX) return NULL;
  copy = malloc(len + 1); if (!copy) return NULL;
  memcpy(copy, str, len); copy[len] = '\0'; return copy;
}

char *hn_path_join(const char *left, const char *right)
{
  size_t ll, rl, slash; char *path;
  if (!left || !right) return NULL;
  ll = strlen(left); rl = strlen(right); slash = !ll || left[ll - 1] != '/';
  if (ll > SIZE_MAX - rl - slash - 1) return NULL;
  path = malloc(ll + slash + rl + 1); if (!path) return NULL;
  memcpy(path, left, ll); if (slash) path[ll++] = '/';
  memcpy(path + ll, right, rl + 1); return path;
}

int hn_str_append(char **str, size_t *len, size_t *cap, const char *suffix, size_t n)
{
  char *new_str; size_t needed, new_cap, suffix_offset = 0; int suffix_is_internal;
  if (!str || !len || !cap || (!suffix && n) || *len > *cap || n > SIZE_MAX - *len - 1) return 0;
  suffix_is_internal = *str && (uintptr_t)suffix >= (uintptr_t)*str &&
                       (uintptr_t)suffix <= (uintptr_t)*str + *len;
  if (suffix_is_internal) {
    suffix_offset = (size_t)(suffix - *str);
    if (n > *len + 1 - suffix_offset) return 0;
  }
  needed = *len + n + 1;
  if (needed > *cap) {
    new_cap = *cap ? *cap : 64;
    while (new_cap < needed) {
      size_t grown = new_cap + (new_cap >> 1);
      if (grown <= new_cap) { new_cap = needed; break; }
      new_cap = grown;
    }
    new_str = realloc(*str, new_cap); if (!new_str) return 0;
    *str = new_str; *cap = new_cap;
  }
  if (suffix_is_internal) suffix = *str + suffix_offset;
  if (n) memmove(*str + *len, suffix, n);
  *len += n; (*str)[*len] = '\0'; return 1;
}

int hn_str_append_utf8(char **str, size_t *len, size_t *cap, unsigned int cp)
{
  struct strbuffer encoded; int ok;
  strbuffer_init(&encoded, 4);
  ok = strbuffer_add_utf8(&encoded, cp) && hn_str_append(str, len, cap, (char *)encoded.data, encoded.len);
  strbuffer_release(&encoded); return ok;
}

int hn_str_append_str(char **str, const char *suffix)
{
  size_t len, cap;
  if (!str || !*str || !suffix) return 0;
  len = strlen(*str); cap = len + 1;
  return hn_str_append(str, &len, &cap, suffix, strlen(suffix));
}
