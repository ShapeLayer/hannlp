#ifndef HANNANUM_CODE_C
#define HANNANUM_CODE_C

#include "code.h"

static const unsigned int choseong_compat[] = {
  0x3131, 0x3132, 0x3134, 0x3137, 0x3138, 0x3139, 0x3141, 0x3142, 0x3143, 0x3145,
  0x3146, 0x3147, 0x3148, 0x3149, 0x314a, 0x314b, 0x314c, 0x314d, 0x314e
};

static const unsigned int jongseong_compat[] = {
  HANNANUM_FILLER, 0x3131, 0x3132, 0x3133, 0x3134, 0x3135, 0x3136, 0x3137, 0x3139, 0x313a,
  0x313b, 0x313c, 0x313d, 0x313e, 0x313f, 0x3140, 0x3141, 0x3142, 0x3144, 0x3145,
  0x3146, 0x3147, 0x3148, 0x314a, 0x314b, 0x314c, 0x314d, 0x314e
};

static const signed char choseong_rev[] = {
  0, 1, -1, 2, -1, -1, 3, 4, 5, -1, -1, -1, -1, -1, -1, -1, 6, 7, 8, -1,
  9, 10, 11, 12, 13, 14, 15, 16, 17, 18
};

static const signed char jongseong_rev[] = {
  1, 2, 3, 4, 5, 6, 7, -1, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, -1, 18,
  19, 20, 21, 22, -1, 23, 24, 25, 26, 27
};

static int
codepoint_vec_push(codepoint_vec_t *vec, unsigned int cp)
{
  unsigned int *next;
  if (vec->count == vec->capacity) {
    size_t new_capacity = vec->capacity == 0 ? 16 : vec->capacity * 2;
    next = (unsigned int *)realloc(vec->items, new_capacity * sizeof(unsigned int));
    if (next == NULL) {
      return 0;
    }
    vec->items = next;
    vec->capacity = new_capacity;
  }
  vec->items[vec->count++] = cp;
  return 1;
}

static void HANNANUM_UNUSED
codepoint_vec_free(codepoint_vec_t *vec)
{
  if (vec == NULL) {
    return;
  }
  free(vec->items);
  vec->items = NULL;
  vec->count = 0;
  vec->capacity = 0;
}

static int
hannanum_code_is_choseong(unsigned int c)
{
  return c >= 0x1100 && c <= 0x1112;
}

static int
hannanum_code_is_jungseong(unsigned int c)
{
  return c >= 0x1161 && c <= 0x1175;
}

static int
hannanum_code_is_jongseong(unsigned int c)
{
  return c >= 0x11a8 && c <= 0x11c2;
}

static unsigned int
hannanum_code_to_jamo(unsigned int index, int flag)
{
  if (flag == 0 && index <= 0x12) {
    return index + 0x1100;
  }
  if (flag == 1 && index <= 0x14) {
    return index + 0x1161;
  }
  if (flag == 2 && index >= 1 && index <= 0x1b) {
    return index + 0x11a7;
  }
  return 0;
}

static unsigned int HANNANUM_UNUSED
hannanum_code_to_compatibility_jamo(unsigned int jamo)
{
  if (jamo >= 0x1100 && jamo < 0x1100 + sizeof(choseong_compat) / sizeof(choseong_compat[0])) {
    return choseong_compat[jamo - 0x1100];
  }
  if (jamo >= 0x1161 && jamo <= 0x1175) {
    return jamo - 0x1161 + 0x314f;
  }
  if (jamo == 0) {
    return HANNANUM_FILLER;
  }
  if (jamo >= 0x11a8 && jamo < 0x11a7 + sizeof(jongseong_compat) / sizeof(jongseong_compat[0])) {
    return jongseong_compat[jamo - 0x11a7];
  }
  return jamo;
}

static int HANNANUM_UNUSED
hannanum_code_to_triple(const char *input, codepoint_vec_t *out)
{
  const unsigned char *p = (const unsigned char *)input;
  while (*p != '\0') {
    unsigned int c;
    size_t width;
    if (!utf8_decode_one(p, &c, &width)) {
      return 0;
    }
    if (c >= 0xac00 && c <= 0xd7af) {
      unsigned int combined = c - 0xac00;
      unsigned int cho = hannanum_code_to_jamo(combined / (21 * 28), 0);
      unsigned int jung;
      unsigned int jong;
      combined %= 21 * 28;
      jung = hannanum_code_to_jamo(combined / 28, 1);
      jong = hannanum_code_to_jamo(combined % 28, 2);
      if (cho != 0 && !codepoint_vec_push(out, cho)) {
        return 0;
      }
      if (jung != 0 && !codepoint_vec_push(out, jung)) {
        return 0;
      }
      if (jong != 0 && !codepoint_vec_push(out, jong)) {
        return 0;
      }
    } else if (c >= 0x3131 && c <= 0x314e) {
      unsigned int idx = c - 0x3131;
      if (jongseong_rev[idx] != -1) {
        if (!codepoint_vec_push(out, (unsigned int)jongseong_rev[idx] + 0x11a7)) {
          return 0;
        }
      } else if (choseong_rev[idx] != -1) {
        if (!codepoint_vec_push(out, (unsigned int)choseong_rev[idx] + 0x1100)) {
          return 0;
        }
      } else if (!codepoint_vec_push(out, c)) {
        return 0;
      }
    } else if (c >= 0x314f && c <= 0x3163) {
      if (!codepoint_vec_push(out, c - 0x314f + 0x1161)) {
        return 0;
      }
    } else if (c == '^') {
      unsigned int next;
      size_t next_width;
      if (p[width] != '\0' && utf8_decode_one(p + width, &next, &next_width) && next >= 0x3131 && next <= 0x314e) {
        unsigned int idx = next - 0x3131;
        if (choseong_rev[idx] != -1) {
          if (!codepoint_vec_push(out, (unsigned int)choseong_rev[idx] + 0x1100)) {
            return 0;
          }
          p += width + next_width;
          continue;
        }
      }
      if (!codepoint_vec_push(out, c)) {
        return 0;
      }
    } else if (!codepoint_vec_push(out, c)) {
      return 0;
    }
    p += width;
  }
  return 1;
}

static char * HANNANUM_UNUSED
hannanum_code_from_triple(const codepoint_vec_t *triple)
{
  char *out = NULL;
  size_t used = 0;
  size_t capacity = 0;
  size_t i = 0;
  while (i < triple->count) {
    unsigned int c = triple->items[i];
    if (hannanum_code_is_choseong(c)) {
      unsigned int cho = c - 0x1100;
      if (i + 1 < triple->count && hannanum_code_is_jungseong(triple->items[i + 1])) {
        unsigned int jung = triple->items[i + 1] - 0x1161;
        unsigned int jong = 0;
        i += 2;
        if (i < triple->count && hannanum_code_is_jongseong(triple->items[i])) {
          jong = triple->items[i] - 0x11a7;
          i++;
        }
        if (!hn_str_append_utf8(&out, &used, &capacity, 0xac00 + (cho * 21 * 28) + (jung * 28) + jong)) {
          free(out);
          return NULL;
        }
      } else {
        unsigned int compat = choseong_compat[cho];
        if (compat != 0x3143 && compat != 0x3149 && compat != 0x3138) {
          if (!hn_str_append_utf8(&out, &used, &capacity, '^')) {
            free(out);
            return NULL;
          }
        }
        if (!hn_str_append_utf8(&out, &used, &capacity, compat)) {
          free(out);
          return NULL;
        }
        i++;
      }
    } else if (hannanum_code_is_jungseong(c)) {
      if (!hn_str_append_utf8(&out, &used, &capacity, c - 0x1161 + 0x314f)) {
        free(out);
        return NULL;
      }
      i++;
    } else if (hannanum_code_is_jongseong(c)) {
      if (!hn_str_append_utf8(&out, &used, &capacity, jongseong_compat[c - 0x11a7])) {
        free(out);
        return NULL;
      }
      i++;
    } else {
      if (!hn_str_append_utf8(&out, &used, &capacity, c)) {
        free(out);
        return NULL;
      }
      i++;
    }
  }
  if (out == NULL) {
    out = hn_strdup("");
  }
  return out;
}

#endif
