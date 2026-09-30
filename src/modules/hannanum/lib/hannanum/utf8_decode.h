#ifndef KO_NLP_HANNANUM_UTF8_DECODE_H
#define KO_NLP_HANNANUM_UTF8_DECODE_H

#include <stddef.h>

/* Decodes one UTF-8 sequence from a NUL-terminated string. Rejects overlong
   forms, UTF-16 surrogates and code points above U+10FFFF so that invalid
   input is never passed through as if it were valid UTF-8. A truncated
   sequence fails on the NUL terminator, so it never reads past the string.
   Shared by the analyzer library and the R Hangul utilities. */
static int
utf8_decode_one(const unsigned char *s, unsigned int *codepoint, size_t *width)
{
  unsigned int cp;
  if (s[0] < 0x80) {
    *codepoint = s[0];
    *width = 1;
    return 1;
  }
  if ((s[0] & 0xe0) == 0xc0 && (s[1] & 0xc0) == 0x80) {
    cp = ((unsigned int)(s[0] & 0x1f) << 6) | (unsigned int)(s[1] & 0x3f);
    if (cp < 0x80) {
      return 0;
    }
    *codepoint = cp;
    *width = 2;
    return 1;
  }
  if ((s[0] & 0xf0) == 0xe0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80) {
    cp = ((unsigned int)(s[0] & 0x0f) << 12) | ((unsigned int)(s[1] & 0x3f) << 6) | (unsigned int)(s[2] & 0x3f);
    if (cp < 0x800 || (cp >= 0xd800 && cp <= 0xdfff)) {
      return 0;
    }
    *codepoint = cp;
    *width = 3;
    return 1;
  }
  if ((s[0] & 0xf8) == 0xf0 && (s[1] & 0xc0) == 0x80 && (s[2] & 0xc0) == 0x80 && (s[3] & 0xc0) == 0x80) {
    cp = ((unsigned int)(s[0] & 0x07) << 18) | ((unsigned int)(s[1] & 0x3f) << 12) | ((unsigned int)(s[2] & 0x3f) << 6) | (unsigned int)(s[3] & 0x3f);
    if (cp < 0x10000 || cp > 0x10ffff) {
      return 0;
    }
    *codepoint = cp;
    *width = 4;
    return 1;
  }
  return 0;
}

#endif
