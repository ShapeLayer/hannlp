#include <R.h>
#include <Rinternals.h>

#include <stdlib.h>
#include <string.h>

typedef struct {
  char *ptr;
  size_t len;
  size_t cap;
} utf8_buffer_t;

static const unsigned int choseong[] = {
  0x3131, 0x3132, 0x3134, 0x3137, 0x3138, 0x3139, 0x3141, 0x3142, 0x3143,
  0x3145, 0x3146, 0x3147, 0x3148, 0x3149, 0x314a, 0x314b, 0x314c, 0x314d, 0x314e
};

static const unsigned int jungseong[] = {
  0x314f, 0x3150, 0x3151, 0x3152, 0x3153, 0x3154, 0x3155, 0x3156, 0x3157,
  0x3158, 0x3159, 0x315a, 0x315b, 0x315c, 0x315d, 0x315e, 0x315f, 0x3160,
  0x3161, 0x3162, 0x3163
};

static const unsigned int jongseong[] = {
  0, 0x3131, 0x3132, 0x3133, 0x3134, 0x3135, 0x3136, 0x3137, 0x3139, 0x313a,
  0x313b, 0x313c, 0x313d, 0x313e, 0x313f, 0x3140, 0x3141, 0x3142, 0x3144,
  0x3145, 0x3146, 0x3147, 0x3148, 0x314a, 0x314b, 0x314c, 0x314d, 0x314e
};

static int
utf8_decode_one(const unsigned char *s, unsigned int *cp, size_t *width)
{
  if (s[0] < 0x80) {
    *cp = s[0];
    *width = 1;
    return 1;
  }
  if ((s[0] & 0xe0) == 0xc0 && (s[1] & 0xc0) == 0x80) {
    *cp = ((unsigned int)(s[0] & 0x1f) << 6) | (unsigned int)(s[1] & 0x3f);
    *width = 2;
    return 1;
  }
  if ((s[0] & 0xf0) == 0xe0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80) {
    *cp = ((unsigned int)(s[0] & 0x0f) << 12) | ((unsigned int)(s[1] & 0x3f) << 6) | (unsigned int)(s[2] & 0x3f);
    *width = 3;
    return 1;
  }
  if ((s[0] & 0xf8) == 0xf0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80) {
    *cp = ((unsigned int)(s[0] & 0x07) << 18) | ((unsigned int)(s[1] & 0x3f) << 12) | ((unsigned int)(s[2] & 0x3f) << 6) | (unsigned int)(s[3] & 0x3f);
    *width = 4;
    return 1;
  }
  return 0;
}

static int
buf_reserve(utf8_buffer_t *buf, size_t extra)
{
  size_t needed = buf->len + extra + 1;
  char *next;
  if (needed <= buf->cap) {
    return 1;
  }
  if (buf->cap == 0) {
    buf->cap = 64;
  }
  while (buf->cap < needed) {
    buf->cap *= 2;
  }
  next = (char *)realloc(buf->ptr, buf->cap);
  if (next == NULL) {
    free(buf->ptr);
    buf->ptr = NULL;
    return 0;
  }
  buf->ptr = next;
  return 1;
}

static int
buf_append_bytes(utf8_buffer_t *buf, const char *s, size_t len)
{
  if (!buf_reserve(buf, len)) {
    return 0;
  }
  memcpy(buf->ptr + buf->len, s, len);
  buf->len += len;
  buf->ptr[buf->len] = '\0';
  return 1;
}

static int
buf_append_cp(utf8_buffer_t *buf, unsigned int cp)
{
  char out[4];
  size_t len;
  if (cp == 0) {
    return 1;
  }
  if (cp < 0x80) {
    out[0] = (char)cp;
    len = 1;
  } else if (cp < 0x800) {
    out[0] = (char)(0xc0 | (cp >> 6));
    out[1] = (char)(0x80 | (cp & 0x3f));
    len = 2;
  } else if (cp < 0x10000) {
    out[0] = (char)(0xe0 | (cp >> 12));
    out[1] = (char)(0x80 | ((cp >> 6) & 0x3f));
    out[2] = (char)(0x80 | (cp & 0x3f));
    len = 3;
  } else {
    out[0] = (char)(0xf0 | (cp >> 18));
    out[1] = (char)(0x80 | ((cp >> 12) & 0x3f));
    out[2] = (char)(0x80 | ((cp >> 6) & 0x3f));
    out[3] = (char)(0x80 | (cp & 0x3f));
    len = 4;
  }
  return buf_append_bytes(buf, out, len);
}

static int is_hangul_syllable(unsigned int cp) { return cp >= 0xac00 && cp <= 0xd7a3; }
static int is_jaeum(unsigned int cp) { return cp >= 0x3131 && cp <= 0x314e; }
static int is_moeum(unsigned int cp) { return cp >= 0x314f && cp <= 0x3163; }
static int is_jamo(unsigned int cp) { return is_jaeum(cp) || is_moeum(cp); }
static int is_hangul(unsigned int cp) { return is_hangul_syllable(cp) || is_jamo(cp); }

static int
index_of_cp(const unsigned int *items, size_t count, unsigned int cp)
{
  size_t i;
  for (i = 0; i < count; i++) {
    if (items[i] == cp) {
      return (int)i;
    }
  }
  return -1;
}

static int chosung_index(unsigned int cp) { return index_of_cp(choseong, sizeof(choseong) / sizeof(choseong[0]), cp); }
static int jungseong_index(unsigned int cp) { return index_of_cp(jungseong, sizeof(jungseong) / sizeof(jungseong[0]), cp); }
static int jongseong_index(unsigned int cp) { return index_of_cp(jongseong, sizeof(jongseong) / sizeof(jongseong[0]), cp); }

static unsigned int
compose_syllable(unsigned int cho, unsigned int jung, unsigned int jong)
{
  int cho_i = chosung_index(cho);
  int jung_i = jungseong_index(jung);
  int jong_i = jongseong_index(jong);
  if (cho == 0 || jung == 0) {
    return cho != 0 ? cho : jung;
  }
  if (cho_i < 0 || jung_i < 0 || jong_i < 0) {
    return 0;
  }
  return 0xac00u + (unsigned int)cho_i * 588u + (unsigned int)jung_i * 28u + (unsigned int)jong_i;
}

static int
string_matches_kind(const char *s, const char *kind)
{
  const unsigned char *p = (const unsigned char *)s;
  if (*p == '\0') {
    return 0;
  }
  while (*p != '\0') {
    unsigned int cp;
    size_t width;
    int ok;
    if (!utf8_decode_one(p, &cp, &width)) {
      return 0;
    }
    if (strcmp(kind, "hangul") == 0) {
      ok = is_hangul(cp);
    } else if (strcmp(kind, "jamo") == 0) {
      ok = is_jamo(cp);
    } else if (strcmp(kind, "jaeum") == 0) {
      ok = is_jaeum(cp);
    } else if (strcmp(kind, "moeum") == 0) {
      ok = is_moeum(cp);
    } else if (strcmp(kind, "ascii") == 0) {
      ok = cp < 128;
    } else {
      error("unsupported Hangul predicate: %s", kind);
      ok = 0;
    }
    if (!ok) {
      return 0;
    }
    p += width;
  }
  return 1;
}

static const char *
jamo_key(unsigned int cp)
{
  switch (cp) {
  case 0x3131: return "r";
  case 0x3132: return "R";
  case 0x3133: return "rt";
  case 0x3134: return "s";
  case 0x3135: return "sw";
  case 0x3136: return "sg";
  case 0x3137: return "e";
  case 0x3138: return "E";
  case 0x3139: return "f";
  case 0x313a: return "fr";
  case 0x313b: return "fa";
  case 0x313c: return "fq";
  case 0x313d: return "ft";
  case 0x313e: return "fx";
  case 0x313f: return "fv";
  case 0x3140: return "fg";
  case 0x3141: return "a";
  case 0x3142: return "q";
  case 0x3143: return "Q";
  case 0x3144: return "qt";
  case 0x3145: return "t";
  case 0x3146: return "T";
  case 0x3147: return "d";
  case 0x3148: return "w";
  case 0x3149: return "W";
  case 0x314a: return "c";
  case 0x314b: return "z";
  case 0x314c: return "x";
  case 0x314d: return "v";
  case 0x314e: return "g";
  case 0x314f: return "k";
  case 0x3150: return "o";
  case 0x3151: return "i";
  case 0x3152: return "O";
  case 0x3153: return "j";
  case 0x3154: return "p";
  case 0x3155: return "u";
  case 0x3156: return "P";
  case 0x3157: return "h";
  case 0x3158: return "hk";
  case 0x3159: return "ho";
  case 0x315a: return "hl";
  case 0x315b: return "y";
  case 0x315c: return "n";
  case 0x315d: return "nj";
  case 0x315e: return "np";
  case 0x315f: return "nl";
  case 0x3160: return "b";
  case 0x3161: return "m";
  case 0x3162: return "ml";
  case 0x3163: return "l";
  default: return "";
  }
}

static unsigned int
key_to_jamo(unsigned int cp)
{
  switch (cp) {
  case 'r': return 0x3131;
  case 'R': return 0x3132;
  case 's': return 0x3134;
  case 'e': return 0x3137;
  case 'E': return 0x3138;
  case 'f': return 0x3139;
  case 'a': return 0x3141;
  case 'q': return 0x3142;
  case 'Q': return 0x3143;
  case 't': return 0x3145;
  case 'T': return 0x3146;
  case 'd': return 0x3147;
  case 'w': return 0x3148;
  case 'W': return 0x3149;
  case 'c': return 0x314a;
  case 'z': return 0x314b;
  case 'x': return 0x314c;
  case 'v': return 0x314d;
  case 'g': return 0x314e;
  case 'k': return 0x314f;
  case 'o': return 0x3150;
  case 'i': return 0x3151;
  case 'O': return 0x3152;
  case 'j': return 0x3153;
  case 'p': return 0x3154;
  case 'u': return 0x3155;
  case 'P': return 0x3156;
  case 'h': return 0x3157;
  case 'y': return 0x315b;
  case 'n': return 0x315c;
  case 'b': return 0x3160;
  case 'm': return 0x3161;
  case 'l': return 0x3163;
  default: return 0;
  }
}

static unsigned int
key_pair_to_jamo(const char *pair)
{
  if (strcmp(pair, "rt") == 0) return 0x3133;
  if (strcmp(pair, "sw") == 0) return 0x3135;
  if (strcmp(pair, "sg") == 0) return 0x3136;
  if (strcmp(pair, "fr") == 0) return 0x313a;
  if (strcmp(pair, "fa") == 0) return 0x313b;
  if (strcmp(pair, "fq") == 0) return 0x313c;
  if (strcmp(pair, "ft") == 0) return 0x313d;
  if (strcmp(pair, "fx") == 0) return 0x313e;
  if (strcmp(pair, "fv") == 0) return 0x313f;
  if (strcmp(pair, "fg") == 0) return 0x3140;
  if (strcmp(pair, "qt") == 0) return 0x3144;
  if (strcmp(pair, "hk") == 0) return 0x3158;
  if (strcmp(pair, "ho") == 0) return 0x3159;
  if (strcmp(pair, "hl") == 0) return 0x315a;
  if (strcmp(pair, "nj") == 0) return 0x315d;
  if (strcmp(pair, "np") == 0) return 0x315e;
  if (strcmp(pair, "nl") == 0) return 0x315f;
  if (strcmp(pair, "ml") == 0) return 0x3162;
  return 0;
}

static unsigned int
merge_jamos(unsigned int first, unsigned int second)
{
  const char *a = jamo_key(first);
  const char *b = jamo_key(second);
  char pair[3];
  if (a[0] == '\0' || a[1] != '\0' || b[0] == '\0' || b[1] != '\0') {
    return 0;
  }
  pair[0] = a[0];
  pair[1] = b[0];
  pair[2] = '\0';
  return key_pair_to_jamo(pair);
}

static unsigned int
first_part_of_complex_jamo(unsigned int cp)
{
  const char *key = jamo_key(cp);
  char one[2];
  if (key[0] == '\0' || key[1] == '\0' || key[2] != '\0') {
    return 0;
  }
  one[0] = key[0];
  one[1] = '\0';
  return key_pair_to_jamo(one) != 0 ? key_pair_to_jamo(one) : key_to_jamo((unsigned int)(unsigned char)key[0]);
}

static unsigned int
second_part_of_complex_jamo(unsigned int cp)
{
  const char *key = jamo_key(cp);
  if (key[0] == '\0' || key[1] == '\0' || key[2] != '\0') {
    return 0;
  }
  return key_to_jamo((unsigned int)(unsigned char)key[1]);
}

typedef struct {
  int word_valid;
  int force;
  unsigned int cho;
  unsigned int jung;
  unsigned int jong;
  utf8_buffer_t output;
  utf8_buffer_t syllables;
  utf8_buffer_t raw;
} automata_t;

static void automata_clear_comp(automata_t *a) { a->cho = 0; a->jung = 0; a->jong = 0; }

static int
automata_push_comp(automata_t *a)
{
  unsigned int cp;
  if (!(a->cho != 0 && a->jung != 0)) {
    a->word_valid = 0;
  }
  cp = compose_syllable(a->cho, a->jung, a->jong);
  automata_clear_comp(a);
  return cp != 0 && buf_append_cp(&a->syllables, cp);
}

static int
automata_flush(automata_t *a)
{
  int r = 0;
  if (a->cho != 0 || a->jung != 0 || a->jong != 0) {
    if (!automata_push_comp(a)) {
      return -1;
    }
  }
  if (a->force) {
    a->word_valid = 2;
  }
  if (a->raw.len != 0 || a->syllables.len != 0) {
    if (a->word_valid == 1 || a->word_valid == 2) {
      if (!buf_append_bytes(&a->output, a->syllables.ptr == NULL ? "" : a->syllables.ptr, a->syllables.len)) {
        return -1;
      }
      r = a->word_valid == 2 ? 2 : 0;
    } else {
      if (!buf_append_bytes(&a->output, a->raw.ptr == NULL ? "" : a->raw.ptr, a->raw.len)) {
        return -1;
      }
      r = 1;
    }
    a->word_valid = 1;
    a->raw.len = 0;
    if (a->raw.ptr != NULL) a->raw.ptr[0] = '\0';
    a->syllables.len = 0;
    if (a->syllables.ptr != NULL) a->syllables.ptr[0] = '\0';
  }
  return r;
}

static int
automata_feed_jamo(automata_t *a, unsigned int cp)
{
  if (!buf_append_cp(&a->raw, cp)) {
    return 0;
  }
  if (is_jamo(cp)) {
    if (is_jaeum(cp)) {
      if (a->cho == 0) {
        if (a->jung != 0 || a->jong != 0) {
          if (a->force) {
            if (!automata_push_comp(a)) return 0;
            a->cho = cp;
          } else {
            a->word_valid = 0;
          }
        } else {
          a->cho = cp;
        }
      } else if (a->jung == 0) {
        if (a->jong != 0) {
          a->word_valid = 0;
        } else {
          if (!automata_push_comp(a)) return 0;
          a->cho = cp;
        }
      } else if (a->jong == 0) {
        if (jongseong_index(cp) < 0) {
          if (!automata_push_comp(a)) return 0;
          a->cho = cp;
        } else {
          a->jong = cp;
        }
      } else {
        unsigned int merged = merge_jamos(a->jong, cp);
        const char *jong_key = jamo_key(a->jong);
        if (jong_key[0] != '\0' && jong_key[1] == '\0' && merged != 0) {
          a->jong = merged;
        } else {
          if (!automata_push_comp(a)) return 0;
          a->cho = cp;
        }
      }
    } else {
      if (a->jong == 0) {
        if (a->jung == 0) {
          a->jung = cp;
        } else {
          unsigned int merged = merge_jamos(a->jung, cp);
          if (merged != 0) {
            a->jung = merged;
          } else {
            if (!automata_push_comp(a)) return 0;
            a->jung = cp;
          }
        }
      } else {
        unsigned int old_jong = a->jong;
        const char *jong_key = jamo_key(old_jong);
        if (jong_key[0] != '\0' && jong_key[1] != '\0' && jong_key[2] == '\0') {
          a->jong = first_part_of_complex_jamo(old_jong);
          if (!automata_push_comp(a)) return 0;
          a->cho = second_part_of_complex_jamo(old_jong);
          a->jung = cp;
        } else {
          a->jong = 0;
          if (!automata_push_comp(a)) return 0;
          a->cho = old_jong;
          a->jung = cp;
        }
      }
    }
  } else {
    int flushed = automata_flush(a);
    if (flushed < 0) return 0;
    if (flushed == 0 || flushed == 2) {
      if (!buf_append_cp(&a->output, cp)) return 0;
    }
  }
  return 1;
}

static int
append_decomposed_jamos(utf8_buffer_t *buf, unsigned int cp)
{
  if (is_hangul_syllable(cp)) {
    unsigned int index = cp - 0xac00;
    unsigned int cho = index / 588;
    unsigned int jung = (index % 588) / 28;
    unsigned int jong = index % 28;
    return buf_append_cp(buf, choseong[cho]) && buf_append_cp(buf, jungseong[jung]) && buf_append_cp(buf, jongseong[jong]);
  }
  if (is_jamo(cp)) {
    return buf_append_cp(buf, cp);
  }
  return 0;
}

static int
append_keystrokes_for_jamo(utf8_buffer_t *buf, unsigned int cp, int fullwidth)
{
  const char *keys = jamo_key(cp);
  size_t i;
  for (i = 0; keys[i] != '\0'; i++) {
    if (fullwidth) {
      if (!buf_append_cp(buf, ((unsigned int)(unsigned char)keys[i] - 0x41u) + 0xff21u)) {
        return 0;
      }
    } else if (!buf_append_bytes(buf, &keys[i], 1)) {
      return 0;
    }
  }
  return 1;
}

SEXP
hannlp_hangul_is(SEXP input_sexp, SEXP kind_sexp)
{
  const char *kind;
  R_xlen_t i;
  SEXP out;
  if (!isString(input_sexp) || !isString(kind_sexp) || XLENGTH(kind_sexp) != 1) {
    error("invalid Hangul predicate input");
  }
  kind = translateCharUTF8(STRING_ELT(kind_sexp, 0));
  PROTECT(out = allocVector(LGLSXP, XLENGTH(input_sexp)));
  for (i = 0; i < XLENGTH(input_sexp); i++) {
    LOGICAL(out)[i] = string_matches_kind(translateCharUTF8(STRING_ELT(input_sexp, i)), kind);
  }
  UNPROTECT(1);
  return out;
}

SEXP
hannlp_hangul_to_jamos(SEXP input_sexp)
{
  const unsigned char *p;
  utf8_buffer_t buf = {0};
  SEXP out;
  if (!isString(input_sexp) || XLENGTH(input_sexp) != 1) {
    error("input must be a character scalar");
  }
  p = (const unsigned char *)translateCharUTF8(STRING_ELT(input_sexp, 0));
  while (*p != '\0') {
    unsigned int cp;
    size_t width;
    if (!utf8_decode_one(p, &cp, &width)) {
      free(buf.ptr);
      error("invalid UTF-8 input");
    }
    if (is_hangul(cp)) {
      if (!append_decomposed_jamos(&buf, cp)) {
        free(buf.ptr);
        error("failed to convert Hangul to Jamos");
      }
    } else if (!buf_append_bytes(&buf, (const char *)p, width)) {
      error("out of memory");
    }
    if (!buf_append_cp(&buf, 0xff5c)) {
      error("out of memory");
    }
    p += width;
  }
  PROTECT(out = mkString(buf.ptr == NULL ? "" : buf.ptr));
  SET_STRING_ELT(out, 0, mkCharCE(buf.ptr == NULL ? "" : buf.ptr, CE_UTF8));
  free(buf.ptr);
  UNPROTECT(1);
  return out;
}

SEXP
hannlp_hangul_to_keystrokes(SEXP input_sexp, SEXP fullwidth_sexp)
{
  const unsigned char *p;
  int fullwidth = asLogical(fullwidth_sexp);
  utf8_buffer_t buf = {0};
  SEXP out;
  if (!isString(input_sexp) || XLENGTH(input_sexp) != 1) {
    error("input must be a character scalar");
  }
  if (fullwidth == NA_LOGICAL) {
    fullwidth = 1;
  }
  p = (const unsigned char *)translateCharUTF8(STRING_ELT(input_sexp, 0));
  while (*p != '\0') {
    unsigned int cp;
    size_t width;
    if (!utf8_decode_one(p, &cp, &width)) {
      free(buf.ptr);
      error("invalid UTF-8 input");
    }
    if (is_hangul_syllable(cp)) {
      unsigned int index = cp - 0xac00;
      unsigned int cho = index / 588;
      unsigned int jung = (index % 588) / 28;
      unsigned int jong = index % 28;
      if (!append_keystrokes_for_jamo(&buf, choseong[cho], fullwidth) || !append_keystrokes_for_jamo(&buf, jungseong[jung], fullwidth) || !append_keystrokes_for_jamo(&buf, jongseong[jong], fullwidth)) {
        free(buf.ptr);
        error("failed to convert Hangul to keystrokes");
      }
    } else if (is_jamo(cp)) {
      if (!append_keystrokes_for_jamo(&buf, cp, fullwidth)) {
        free(buf.ptr);
        error("failed to convert Jamo to keystrokes");
      }
    } else if (!buf_append_bytes(&buf, (const char *)p, width)) {
      error("out of memory");
    }
    if (!buf_append_cp(&buf, 0xff5c)) {
      error("out of memory");
    }
    p += width;
  }
  PROTECT(out = mkString(buf.ptr == NULL ? "" : buf.ptr));
  SET_STRING_ELT(out, 0, mkCharCE(buf.ptr == NULL ? "" : buf.ptr, CE_UTF8));
  free(buf.ptr);
  UNPROTECT(1);
  return out;
}

SEXP
hannlp_hangul_automata(SEXP input_sexp, SEXP keystroke_sexp, SEXP force_sexp)
{
  const unsigned char *p;
  int keystroke = asLogical(keystroke_sexp);
  int force = asLogical(force_sexp);
  automata_t automata;
  int flushed;
  SEXP out;

  if (!isString(input_sexp) || XLENGTH(input_sexp) != 1) {
    error("input must be a character scalar");
  }
  if (keystroke == NA_LOGICAL) {
    keystroke = 0;
  }
  if (force == NA_LOGICAL) {
    force = 0;
  }

  memset(&automata, 0, sizeof(automata));
  automata.word_valid = 1;
  automata.force = force ? 1 : 0;

  p = (const unsigned char *)translateCharUTF8(STRING_ELT(input_sexp, 0));
  while (*p != '\0') {
    unsigned int cp;
    size_t width;
    if (!utf8_decode_one(p, &cp, &width)) {
      free(automata.output.ptr);
      free(automata.syllables.ptr);
      free(automata.raw.ptr);
      error("invalid UTF-8 input");
    }
    if (keystroke) {
      unsigned int jamo = width == 1 ? key_to_jamo(cp) : 0;
      cp = jamo == 0 ? cp : jamo;
    }
    if (!automata_feed_jamo(&automata, cp)) {
      free(automata.output.ptr);
      free(automata.syllables.ptr);
      free(automata.raw.ptr);
      error("failed to compose Hangul input");
    }
    p += width;
  }

  flushed = automata_flush(&automata);
  if (flushed < 0) {
    free(automata.output.ptr);
    free(automata.syllables.ptr);
    free(automata.raw.ptr);
    error("failed to compose Hangul input");
  }
  if (flushed == 1 && !automata.force) {
    free(automata.output.ptr);
    free(automata.syllables.ptr);
    free(automata.raw.ptr);
    PROTECT(out = mkString(translateCharUTF8(STRING_ELT(input_sexp, 0))));
    SET_STRING_ELT(out, 0, mkCharCE(translateCharUTF8(STRING_ELT(input_sexp, 0)), CE_UTF8));
    UNPROTECT(1);
    return out;
  }

  PROTECT(out = mkString(automata.output.ptr == NULL ? "" : automata.output.ptr));
  SET_STRING_ELT(out, 0, mkCharCE(automata.output.ptr == NULL ? "" : automata.output.ptr, CE_UTF8));
  free(automata.output.ptr);
  free(automata.syllables.ptr);
  free(automata.raw.ptr);
  UNPROTECT(1);
  return out;
}
