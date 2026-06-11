static int
is_sentence_symbol(unsigned char c)
{
  switch (c) {
  case ')':
  case ']':
  case '}':
  case '?':
  case '!':
  case '.':
  case '\'':
  case '"':
    return 1;
  default:
    return 0;
  }
}

static int
is_eojeol_space(unsigned char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

static int
find_sentence_segmentor_split(const char *token, size_t len, size_t *prefix_len, size_t *symbol_len)
{
  size_t          j;
  for (j = 0; j < len; j++) {
    unsigned char   c = (unsigned char)token[j];
    if (c == '.') {
      if (j == 1) {
        continue;
      }
      if (j > 0 && isalpha((unsigned char)token[j - 1])) {
        continue;
      }
      if (j + 1 < len && isdigit((unsigned char)token[j + 1]) && memchr(token, ',', j) == NULL) {
        continue;
      }
    } else if (c != '!' && c != '?') {
      continue;
    }
    *prefix_len = j;
    *symbol_len = 1;
    while (j + *symbol_len < len && is_sentence_symbol((unsigned char)token[j + *symbol_len])) {
      (*symbol_len)++;
    }
    return 1;
  }
  return 0;
}
static int
split_eojeols(const char *input, str_vec_t * out)
{
  const char     *p = input;
  while (p != NULL && *p != '\0') {
    const char     *start;
    char           *token;
    size_t          len;
    size_t          split_at = 0;
    size_t          eos_prefix_len = 0;
    size_t          eos_symbol_len = 0;
    int             did_split = 0;
    while (*p != '\0' && is_eojeol_space((unsigned char)*p)) {
      p++;
    }
    if (*p == '\0') {
      break;
    }
    start = p;
    while (*p != '\0' && !is_eojeol_space((unsigned char)*p)) {
      p++;
    }
    len = (size_t) (p - start);
    if (len > 6 && memcmp(start + len - 6, "......", 6) == 0) {
      token = strbuf_substr(start, len - 6);
      if (token == NULL) {
        return 0;
      }
      if (!str_vec_push_owned(out, token)) {
        free(token);
        return 0;
      }
      token = strbuf_substr(start + len - 6, 5);
      if (token == NULL) {
        return 0;
      }
      if (!str_vec_push_owned(out, token)) {
        free(token);
        return 0;
      }
      token = strbuf_substr(start + len - 1, 1);
      if (token == NULL) {
        return 0;
      }
      if (!str_vec_push_owned(out, token)) {
        free(token);
        return 0;
      }
      continue;
    }
    if (find_sentence_segmentor_split(start, len, &eos_prefix_len, &eos_symbol_len)) {
      if (eos_symbol_len == 1 && start[eos_prefix_len] == '.' && eos_prefix_len > 0 && isdigit((unsigned char)start[eos_prefix_len - 1]) && eos_prefix_len + 1 == len) {
        token = strbuf_substr(start, len);
        if (token == NULL) {
          return 0;
        }
        if (!str_vec_push_owned(out, token)) {
          free(token);
          return 0;
        }
        continue;
      }
      if (eos_symbol_len == 1 && start[eos_prefix_len] == '.' && memchr(start, ',', eos_prefix_len) != NULL) {
        token = strbuf_substr(start, eos_prefix_len + 1);
        if (token == NULL) {
          return 0;
        }
        if (!str_vec_push_owned(out, token)) {
          free(token);
          return 0;
        }
        if (eos_prefix_len + 1 < len) {
          token = strbuf_substr(start + eos_prefix_len + 1, len - eos_prefix_len - 1);
          if (token == NULL) {
            return 0;
          }
          if (!str_vec_push_owned(out, token)) {
            free(token);
            return 0;
          }
        }
        continue;
      }
      if (eos_prefix_len > 0) {
        token = strbuf_substr(start, eos_prefix_len);
        if (token == NULL) {
          return 0;
        }
        if (!str_vec_push_owned(out, token)) {
          free(token);
          return 0;
        }
      }
      token = strbuf_substr(start + eos_prefix_len, eos_symbol_len);
      if (token == NULL) {
        return 0;
      }
      if (!str_vec_push_owned(out, token)) {
        free(token);
        return 0;
      }
      if (eos_prefix_len + eos_symbol_len < len) {
        token = strbuf_substr(start + eos_prefix_len + eos_symbol_len, len - eos_prefix_len - eos_symbol_len);
        if (token == NULL) {
          return 0;
        }
        if (!str_vec_push_owned(out, token)) {
          free(token);
          return 0;
        }
      }
      continue;
    }
    if (len > 3 && memcmp(start + len - 3, "...", 3) == 0) {
      token = strbuf_substr(start, len - 3);
      if (token == NULL) {
        return 0;
      }
      if (!str_vec_push_owned(out, token)) {
        free(token);
        return 0;
      }
      token = strbuf_substr(start + len - 3, 3);
      if (token == NULL) {
        return 0;
      }
      if (!str_vec_push_owned(out, token)) {
        free(token);
        return 0;
      }
      continue;
    }
    if (len > 2 && (start[len - 2] == '?' || start[len - 2] == '!') && (start[len - 1] == '?' || start[len - 1] == '!')) {
      token = strbuf_substr(start, len - 2);
      if (token == NULL) {
        return 0;
      }
      if (!str_vec_push_owned(out, token)) {
        free(token);
        return 0;
      }
      token = strbuf_substr(start + len - 2, 2);
      if (token == NULL) {
        return 0;
      }
      if (!str_vec_push_owned(out, token)) {
        free(token);
        return 0;
      }
      continue;
    }
    if (len > 4) {
      size_t          email_dot;
      for (email_dot = 1; email_dot + 1 < len; email_dot++) {
        if (start[email_dot] == '.' && memchr(start, '@', email_dot) != NULL) {
          token = strbuf_substr(start, email_dot + 1);
          if (token == NULL) {
            return 0;
          }
          if (!str_vec_push_owned(out, token)) {
            free(token);
            return 0;
          }
          token = strbuf_substr(start + email_dot + 1, len - email_dot - 1);
          if (token == NULL) {
            return 0;
          }
          if (!str_vec_push_owned(out, token)) {
            free(token);
            return 0;
          }
          email_dot = len;
          break;
        }
      }
      if (email_dot == len) {
        continue;
      }
    }
    for (split_at = 1; split_at + 1 < len; split_at++) {
      size_t          j;
      if (start[split_at] != '.' || !isdigit((unsigned char)start[split_at - 1]) || !isdigit((unsigned char)start[split_at + 1])) {
        continue;
      }
      for (j = split_at + 1; j < len && isdigit((unsigned char)start[j]); j++) {
      }
      if (j < len) {
        token = strbuf_substr(start, split_at + 1);
        if (token == NULL) {
          return 0;
        }
        if (!str_vec_push_owned(out, token)) {
          free(token);
          return 0;
        }
        token = strbuf_substr(start + split_at + 1, len - split_at - 1);
        if (token == NULL) {
          return 0;
        }
        if (!str_vec_push_owned(out, token)) {
          free(token);
          return 0;
        }
        did_split = 1;
        break;
      }
    }
    if (did_split) {
      continue;
    }
    if (len > 1 && strchr(".!?", start[len - 1]) != NULL && !(start[len - 1] == '.' && len > 1 && isalpha((unsigned char)start[len - 2]))) {
      token = strbuf_substr(start, len - 1);
      if (token == NULL) {
        return 0;
      }
      if (!str_vec_push_owned(out, token)) {
        free(token);
        return 0;
      }
      token = strbuf_substr(start + len - 1, 1);
      if (token == NULL) {
        return 0;
      }
      if (!str_vec_push_owned(out, token)) {
        free(token);
        return 0;
      }
      continue;
    }
    token = strbuf_substr(start, len);
    if (token == NULL) {
      return 0;
    }
    if (!str_vec_push_owned(out, token)) {
      free(token);
      return 0;
    }
  }
  return 1;
}
