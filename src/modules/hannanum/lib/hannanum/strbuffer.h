#ifndef HANNANUM_STRBUFFER_H
#define HANNANUM_STRBUFFER_H
#include <stddef.h>

struct strbuffer {
  unsigned char *data;
  size_t len;
  size_t cap;
  int failed;
};

void strbuffer_init(struct strbuffer *buf, size_t initial_capacity);
void strbuffer_release(struct strbuffer *buf);
void strbuffer_reset(struct strbuffer *buf);
int strbuffer_reserve(struct strbuffer *buf, size_t capacity);
int strbuffer_set(struct strbuffer *buf, const unsigned char *data, size_t len);
int strbuffer_set_str(struct strbuffer *buf, const char *str);
int strbuffer_add_byte(struct strbuffer *buf, unsigned char byte);
int strbuffer_add(struct strbuffer *buf, const unsigned char *data, size_t len);
int strbuffer_add_str(struct strbuffer *buf, const char *str);
int strbuffer_add_utf8(struct strbuffer *buf, unsigned int codepoint);
size_t strbuffer_len(const struct strbuffer *buf);
void strbuffer_copy(char *dst, size_t dst_size, const struct strbuffer *src);
void strbuffer_swap(struct strbuffer *left, struct strbuffer *right);
unsigned char *strbuffer_steal(struct strbuffer *buf);
int strbuffer_cmp(const struct strbuffer *left, const struct strbuffer *right);
ptrdiff_t strbuffer_find(const struct strbuffer *buf, int byte, size_t start);
ptrdiff_t strbuffer_rfind(const struct strbuffer *buf, int byte, size_t start);
void strbuffer_truncate(struct strbuffer *buf, size_t len);
void strbuffer_consume(struct strbuffer *buf, size_t len);
void strbuffer_trim_right(struct strbuffer *buf);
void strbuffer_trim(struct strbuffer *buf);
void strbuffer_squash_spaces(struct strbuffer *buf);
void strbuffer_unescape(struct strbuffer *buf);

char *hn_strdup(const char *str);
char *hn_strndup(const char *str, size_t len);
char *hn_path_join(const char *left, const char *right);
int hn_str_append(char **str, size_t *len, size_t *capacity,
                  const char *suffix, size_t suffix_len);
int hn_str_append_utf8(char **str, size_t *len, size_t *capacity,
                       unsigned int codepoint);
int hn_str_append_str(char **str, const char *suffix);
#endif
