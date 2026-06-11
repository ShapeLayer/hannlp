static int
utf8_char_width_from_first(unsigned char c)
{
  if (c < 0x80) {
    return 1;
  }
  if ((c & 0xe0) == 0xc0) {
    return 2;
  }
  if ((c & 0xf0) == 0xe0) {
    return 3;
  }
  if ((c & 0xf8) == 0xf0) {
    return 4;
  }
  return 1;
}

static size_t
utf8_char_count(const char *s, size_t len)
{
  size_t pos = 0;
  size_t count = 0;
  while (pos < len) {
    size_t width = (size_t)utf8_char_width_from_first((unsigned char)s[pos]);
    if (pos + width > len) {
      width = len - pos;
    }
    pos += width;
    count++;
  }
  return count;
}

static int
filter_informal_word(const char *word, size_t len, char **buffer, size_t *used, size_t *capacity)
{
  size_t pos = 0;
  const char *check_start;
  size_t check_len;
  int repeat_count = 0;
  if (len == 0) {
    return 1;
  }
  check_start = word;
  check_len = (size_t)utf8_char_width_from_first((unsigned char)word[0]);
  if (!strbuf_append_to_cstr(buffer, used, capacity, check_start, check_len)) {
    return 0;
  }
  pos = check_len;
  while (pos < len) {
    const char *current = word + pos;
    size_t current_len = (size_t)utf8_char_width_from_first((unsigned char)*current);
    if (pos + current_len > len) {
      current_len = len - pos;
    }
    if (current_len == check_len && memcmp(check_start, current, current_len) == 0) {
      if (repeat_count == 4) {
        if (!strbuf_append_to_cstr(buffer, used, capacity, " ", 1)) {
          return 0;
        }
        if (!strbuf_append_to_cstr(buffer, used, capacity, current, current_len)) {
          return 0;
        }
        repeat_count = 0;
      } else {
        if (!strbuf_append_to_cstr(buffer, used, capacity, current, current_len)) {
          return 0;
        }
        repeat_count++;
      }
    } else {
      if (check_len == 1 && check_start[0] == '.') {
        if (!strbuf_append_to_cstr(buffer, used, capacity, " ", 1)) {
          return 0;
        }
      }
      if (!strbuf_append_to_cstr(buffer, used, capacity, current, current_len)) {
        return 0;
      }
      check_start = current;
      check_len = current_len;
      repeat_count = 0;
    }
    pos += current_len;
  }
  return 1;
}

static char *
informal_sentence_filter(const char *input)
{
  const char *p = input;
  char *buffer = NULL;
  size_t used = 0;
  size_t capacity = 0;
  if (input == NULL) {
    return NULL;
  }
  while (*p != '\0') {
    const char *start;
    size_t len;
    while (*p == ' ' || *p == '\t') {
      p++;
    }
    if (*p == '\0') {
      break;
    }
    start = p;
    while (*p != '\0' && *p != ' ' && *p != '\t') {
      p++;
    }
    len = (size_t)(p - start);
    if (utf8_char_count(start, len) > 5) {
      if (!filter_informal_word(start, len, &buffer, &used, &capacity)) {
        free(buffer);
        return NULL;
      }
    } else if (!strbuf_append_to_cstr(&buffer, &used, &capacity, start, len)) {
      free(buffer);
      return NULL;
    }
    if (!strbuf_append_to_cstr(&buffer, &used, &capacity, " ", 1)) {
      free(buffer);
      return NULL;
    }
  }
  if (buffer == NULL) {
    buffer = strbuf_strdup("");
  }
  return buffer;
}
