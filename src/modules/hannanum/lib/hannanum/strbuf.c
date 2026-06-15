/*
buffer.h(strbuf.h), buffer.c(strbuf.c), chunk.h

are derived from Copyright (c) 2014, John MacFarlane.
(derived from "https://github.com/commonmark/cmark")

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

/*
 * This file keeps the upstream cmark-style buffer API small and predictable.
 * Parser and renderer code store byte ranges in strbuf and track lengths
 * explicitly, which is important because NamuMark documents are UTF-8 text but
 * many parser decisions are byte-oriented delimiter scans.
 */

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strbuf.h"

static int strbuf_is_ascii_space(unsigned char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

#ifndef MIN
#define MIN(x, y) ((x < y) ? x : y)
#endif

unsigned char strbuf__init_buf[1] = {0};

void strbuf_init(strbuf *buf, bufsize_t init_size) {
  buf->ptr = strbuf__init_buf;
  buf->asize = 0;
  buf->size = 0;

  if (init_size > 0) {
    strbuf_grow(buf, init_size);
  }
}

static void S_strbuf_grow_by(strbuf *buf, bufsize_t grow_size) {
  strbuf_grow(buf, buf->size + grow_size);
}

void strbuf_grow(strbuf *buf, bufsize_t grow_size) {
  assert(grow_size > 0);

  if (grow_size < buf->asize) { return; }

  if (grow_size > (bufsize_t)(BUFSIZE_MAX / 2)) {
    strbuf_free(buf);
    return;
  }

  bufsize_t new_size = grow_size + grow_size / 2;
  new_size += 1;
  new_size = (new_size + 7) & ~7;

  buf->ptr = (unsigned char *)realloc(buf->asize ? buf->ptr : NULL, new_size);
  buf->asize = new_size;
}

bufsize_t strbuf_len(const strbuf *buf) { return buf->size; }

void strbuf_free(strbuf *buf) {
  if (!buf) { return; }

  if (buf->ptr != strbuf__init_buf) {
    free(buf->ptr);
  }

  strbuf_init(buf, 0);
}

void strbuf_clear(strbuf *buf) {
  buf->size = 0;

  if (buf->asize > 0) {
    buf->ptr[0] = '\0';
  }
}

void strbuf_set(strbuf *buf, const unsigned char *data, bufsize_t len) {
  if (len <= 0 || data == NULL) {
    strbuf_clear(buf);
  } else {
    if (data != buf->ptr) {
      if (len >= buf->asize) {
        strbuf_grow(buf, len);
      }
      memmove(buf->ptr, data, len);
    }
    buf->size = len;
    buf->ptr[buf->size] = '\0';
  }
}

void strbuf_sets(strbuf *buf, const char *str) {
  strbuf_set(buf, (const unsigned char *)str, str ? (bufsize_t)strlen(str) : 0);
}

void strbuf_putc(strbuf *buf, unsigned char c) {
  S_strbuf_grow_by(buf, 1);
  buf->ptr[buf->size++] = c;
  buf->ptr[buf->size] = '\0';
}

void strbuf_put(strbuf *buf, const unsigned char *data, bufsize_t len) {
  if (len <= 0) { return; }

  S_strbuf_grow_by(buf, len);
  memmove(buf->ptr + buf->size, data, len);
  buf->size += len;
  buf->ptr[buf->size] = '\0';
}

void strbuf_puts(strbuf *buf, const char *str) {
  strbuf_put(buf, (const unsigned char *)str, (bufsize_t)strlen(str));
}

int strbuf_put_utf8_codepoint(strbuf *buf, unsigned int cp) {
  unsigned char bytes[4];
  bufsize_t len;
  if (cp < 0x80) {
    bytes[0] = (unsigned char)cp;
    len = 1;
  } else if (cp < 0x800) {
    bytes[0] = (unsigned char)(0xc0 | (cp >> 6));
    bytes[1] = (unsigned char)(0x80 | (cp & 0x3f));
    len = 2;
  } else if (cp < 0x10000) {
    bytes[0] = (unsigned char)(0xe0 | (cp >> 12));
    bytes[1] = (unsigned char)(0x80 | ((cp >> 6) & 0x3f));
    bytes[2] = (unsigned char)(0x80 | (cp & 0x3f));
    len = 3;
  } else {
    bytes[0] = (unsigned char)(0xf0 | (cp >> 18));
    bytes[1] = (unsigned char)(0x80 | ((cp >> 12) & 0x3f));
    bytes[2] = (unsigned char)(0x80 | ((cp >> 6) & 0x3f));
    bytes[3] = (unsigned char)(0x80 | (cp & 0x3f));
    len = 4;
  }
  strbuf_put(buf, bytes, len);
  return 1;
}

void strbuf_copy_cstr(char *dest, bufsize_t dest_size, const strbuf *src) {
  assert(src);
  if (!dest || dest_size <= 0) { return; }

  if (src->size == 0 || src->asize <= 0) {
    dest[0] = '\0';
    return;
  }

  bufsize_t copylen = src->size;
  if (copylen > dest_size - 1) {
    copylen = dest_size - 1;
  }
  memmove(dest, src->ptr, copylen);
  dest[copylen] = '\0';
}

void strbuf_swap(strbuf *a, strbuf *b) {
  strbuf tmp = *a;
  *a = *b;
  *b = tmp;
}

unsigned char *strbuf_detach(strbuf *buf) {
  unsigned char *data = buf->ptr;

  if (buf->asize == 0) {
    return (unsigned char *)calloc(1, 1);
  }

  strbuf_init(buf, 0);
  return data;
}

int strbuf_cmp(const strbuf *a, const strbuf *b) {
  int result = memcmp(a->ptr, b->ptr, MIN(a->size, b->size));
  if (result != 0) { return result; }
  if (a -> size < b -> size) { return -1; }
  if (a -> size > b -> size) { return 1; }
  return 0;
}

bufsize_t strbuf_strchr(const strbuf *buf, int c, bufsize_t pos) {
  if (pos >= buf->size) { return -1; }
  if (pos < 0) { pos = 0; }

  const unsigned char *p = (unsigned char *)memchr(buf->ptr + pos, c, buf->size - pos);
  if (!p) { return -1; }

  return (bufsize_t)(p - buf->ptr);
}

bufsize_t strbuf_strrchr(const strbuf *buf, int c, bufsize_t pos) {
  if (pos < 0 || buf->size == 0) { return -1; }
  if (pos >= buf->size) { pos = buf->size - 1; }

  bufsize_t i;
  for (i = pos; i >= 0; i--) {
    if (buf->ptr[i] == (unsigned char)c) {
      return i;
    }
  }

  return -1;
}

void strbuf_truncate(strbuf *buf, bufsize_t len) {
  if (len < 0) { len = 0; }

  if (len < buf->size) {
    buf->ptr[len] = '\0';
    buf->size = len;
  }
}

void strbuf_drop(strbuf *buf, bufsize_t n) {
  if (n > 0) {
    if (n > buf->size) { n = buf->size; }
    buf->size = buf->size - n;
    if (buf->size) {
      memmove(buf->ptr, buf->ptr + n, buf->size);
    }
    buf->ptr[buf->size] = '\0';
  }
}

void strbuf_rtrim(strbuf *buf) {
  if (!buf->size) { return; }

  while (buf->size > 0) {
    if (!strbuf_is_ascii_space((unsigned char)buf->ptr[buf->size - 1])) { break; }
    buf->size--;
  }

  buf->ptr[buf->size] = '\0';
}

void strbuf_trim(strbuf *buf) {
  bufsize_t i = 0;

  if (!buf->size) { return; }

  while (i < buf->size && strbuf_is_ascii_space((unsigned char)buf->ptr[i])) { i++; }

  strbuf_drop(buf, i);
  strbuf_rtrim(buf);
}

void strbuf_normalize_whitespace(strbuf *buf) {
  bool last_char_was_space = false;
  bufsize_t r, w;

  for (r = 0, w = 0; r < buf->size; r++) {
    if (strbuf_is_ascii_space((unsigned char)buf->ptr[r])) {
      if (!last_char_was_space && w > 0) {
        buf->ptr[w++] = ' ';
        last_char_was_space = true;
      }
    } else {
      buf->ptr[w++] = buf->ptr[r];
      last_char_was_space = false;
    }
  }

  if (w > 0 && buf->ptr[w - 1] == ' ') {
    w--;
  }

  strbuf_truncate(buf, w);
}

void strbuf_unescape(strbuf *buf) {
  bufsize_t r, w;
  for (r = 0, w = 0; r < buf->size; r++) {
    if (buf->ptr[r] == '\\' && ispunct(buf->ptr[r + 1])) { r++; }
    buf->ptr[w++] = buf->ptr[r];
  }

  strbuf_truncate(buf, w);
}

char *strbuf_strdup(const char *str) {
  strbuf buf;
  if (str == NULL) { return NULL; }
  strbuf_init(&buf, (bufsize_t)strlen(str));
  strbuf_puts(&buf, str);
  return (char *)strbuf_detach(&buf);
}

char *strbuf_substr(const char *start, size_t len) {
  strbuf buf;
  if (start == NULL) { return NULL; }
  strbuf_init(&buf, (bufsize_t)len);
  strbuf_put(&buf, (const unsigned char *)start, (bufsize_t)len);
  return (char *)strbuf_detach(&buf);
}

char *strbuf_join_path(const char *a, const char *b) {
  strbuf buf;
  size_t an;
  if (a == NULL || b == NULL) { return NULL; }
  an = strlen(a);
  strbuf_init(&buf, (bufsize_t)(an + strlen(b) + 1));
  strbuf_puts(&buf, a);
  if (an == 0 || a[an - 1] != '/') {
    strbuf_putc(&buf, '/');
  }
  strbuf_puts(&buf, b);
  return (char *)strbuf_detach(&buf);
}

int strbuf_append_to_cstr(char **buffer, size_t *used, size_t *capacity, const char *data, size_t len) {
  strbuf buf;
  if (buffer == NULL || used == NULL || capacity == NULL || data == NULL) { return 0; }
  strbuf_init(&buf, (bufsize_t)(*capacity > *used ? *capacity : *used + len));
  if (*buffer != NULL && *used > 0) {
    strbuf_put(&buf, (const unsigned char *)*buffer, (bufsize_t)*used);
  }
  strbuf_put(&buf, (const unsigned char *)data, (bufsize_t)len);
  free(*buffer);
  *used = (size_t)strbuf_len(&buf);
  *capacity = (size_t)buf.asize;
  *buffer = (char *)strbuf_detach(&buf);
  return 1;
}

int strbuf_append_utf8_to_cstr(char **buffer, size_t *used, size_t *capacity, unsigned int cp) {
  strbuf buf;
  if (buffer == NULL || used == NULL || capacity == NULL) { return 0; }
  strbuf_init(&buf, (bufsize_t)(*capacity > *used ? *capacity : *used + 4));
  if (*buffer != NULL && *used > 0) {
    strbuf_put(&buf, (const unsigned char *)*buffer, (bufsize_t)*used);
  }
  strbuf_put_utf8_codepoint(&buf, cp);
  free(*buffer);
  *used = (size_t)strbuf_len(&buf);
  *capacity = (size_t)buf.asize;
  *buffer = (char *)strbuf_detach(&buf);
  return 1;
}

int strbuf_append_joined(char **left, const char *right) {
  strbuf joined;
  if (left == NULL || *left == NULL || right == NULL) { return 0; }
  strbuf_init(&joined, (bufsize_t)(strlen(*left) + strlen(right)));
  strbuf_puts(&joined, *left);
  strbuf_puts(&joined, right);
  free(*left);
  *left = (char *)strbuf_detach(&joined);
  return 1;
}
