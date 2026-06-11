static int
utf8_decode_one(const unsigned char *s, unsigned int *codepoint, size_t *width)
{
  if (s[0] < 0x80) {
    *codepoint = s[0];
    *width = 1;
    return 1;
  }
  if ((s[0] & 0xe0) == 0xc0 && (s[1] & 0xc0) == 0x80) {
    *codepoint = ((unsigned int)(s[0] & 0x1f) << 6) | (unsigned int)(s[1] & 0x3f);
    *width = 2;
    return 1;
  }
  if ((s[0] & 0xf0) == 0xe0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80) {
    *codepoint = ((unsigned int)(s[0] & 0x0f) << 12) | ((unsigned int)(s[1] & 0x3f) << 6) | (unsigned int)(s[2] & 0x3f);
    *width = 3;
    return 1;
  }
  if ((s[0] & 0xf8) == 0xf0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80) {
    *codepoint = ((unsigned int)(s[0] & 0x07) << 18) | ((unsigned int)(s[1] & 0x3f) << 12) | ((unsigned int)(s[2] & 0x3f) << 6) | (unsigned int)(s[3] & 0x3f);
    *width = 4;
    return 1;
  }
  return 0;
}

static int
last_hangul_syllable(const char *s, unsigned int *syllable)
{
  const unsigned char *p = (const unsigned char *)s;
  unsigned int    current = 0;
  size_t          width = 0;
  int             found = 0;
  while (*p != '\0') {
    if (!utf8_decode_one(p, &current, &width)) {
      return 0;
    }
    if (current >= 0xac00 && current <= 0xd7a3) {
      *syllable = current;
      found = 1;
    }
    p += width;
  }
  return found;
}

static int HANNANUM_UNUSED
hangul_has_positive_vowel(const char *s)
{
  unsigned int    syllable;
  unsigned int    index;
  unsigned int    vowel;
  if (!last_hangul_syllable(s, &syllable)) {
    return 0;
  }
  index = syllable - 0xac00;
  vowel = (index / 28) % 21;
  return vowel == 0 || vowel == 2 || vowel == 8;
}

static int HANNANUM_UNUSED
hangul_final_is_vowel_or_l(const char *s)
{
  unsigned int    syllable;
  unsigned int    final_index;
  if (!last_hangul_syllable(s, &syllable)) {
    return 0;
  }
  final_index = (syllable - 0xac00) % 28;
  return final_index == 0 || final_index == 8;
}

static unsigned long HANNANUM_UNUSED
hash_string(const char *s)
{
  unsigned long   h = 5381;
  unsigned char   c;
  while ((c = (unsigned char)*s++) != 0) {
    h = ((h << 5) + h) + c;
  }
  return h;
}

static char * HANNANUM_UNUSED
trim(char *s)
{
  char           *end;
  while (*s != '\0' && isspace((unsigned char)*s)) {
    s++;
  }
  end = s + strlen(s);
  while (end > s && isspace((unsigned char)end[-1])) {
    *--end = '\0';
  }
  return s;
}

static int HANNANUM_UNUSED
starts_with(const char *s, const char *prefix)
{
  return strncmp(s, prefix, strlen(prefix)) == 0;
}

static void HANNANUM_UNUSED
set_error(hannanum_t * h, const char *message)
{
  if (h != NULL && message != NULL) {
    snprintf(h->error, sizeof(h->error), "%s", message);
  }
}
