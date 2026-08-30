#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define HANNANUM_UNUSED __attribute__((unused))
#else
#define HANNANUM_UNUSED
#endif

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

#include "strbuffer.c"
#include "code.c"

static int
expect_round_trip(const char *input)
{
  codepoint_vec_t triple;
  char *round_trip;
  memset(&triple, 0, sizeof(triple));
  if (!hannanum_code_to_triple(input, &triple)) {
    codepoint_vec_free(&triple);
    return 0;
  }
  round_trip = hannanum_code_from_triple(&triple);
  codepoint_vec_free(&triple);
  if (round_trip == NULL) {
    return 0;
  }
  if (strcmp(input, round_trip) != 0) {
    fprintf(stderr, "round trip mismatch: %s -> %s\n", input, round_trip);
    free(round_trip);
    return 0;
  }
  free(round_trip);
  return 1;
}

static int
expect_buffer_failure_is_sticky(void)
{
  struct strbuffer buf;
  unsigned char *result;

  strbuffer_init(&buf, 0);
  if (strbuffer_reserve(&buf, SIZE_MAX) || strbuffer_add_str(&buf, "partial")) {
    strbuffer_release(&buf);
    return 0;
  }
  result = strbuffer_steal(&buf);
  free(result);
  return result == NULL;
}

static int
expect_self_append_at_terminator(void)
{
  struct strbuffer buf;
  char *str;
  size_t len = 3;
  size_t cap = 4;

  strbuffer_init(&buf, 0);
  while (buf.len < 63 && strbuffer_add_byte(&buf, 'a')) {}
  if (buf.len != 63 || !strbuffer_add(&buf, buf.data + buf.len, 1) ||
      buf.len != 64 || buf.data[63] != '\0' || buf.data[64] != '\0') {
    strbuffer_release(&buf);
    return 0;
  }
  strbuffer_release(&buf);

  str = malloc(cap);
  if (str == NULL) return 0;
  memcpy(str, "abc", cap);
  if (!hn_str_append(&str, &len, &cap, str + len, 1) || len != 4 ||
      str[3] != '\0' || str[4] != '\0') {
    free(str);
    return 0;
  }
  free(str);
  return 1;
}

int
main(void)
{
  if (!expect_round_trip("학교")) {
    return 1;
  }
  if (!expect_round_trip("가진")) {
    return 1;
  }
  if (!expect_round_trip("Coex 12.42")) {
    return 1;
  }
  if (!expect_buffer_failure_is_sticky()) {
    fprintf(stderr, "buffer failure was not propagated\n");
    return 1;
  }
  if (!expect_self_append_at_terminator()) {
    fprintf(stderr, "self append at terminator failed\n");
    return 1;
  }
  if (!hannanum_code_is_choseong(0x1100) || !hannanum_code_is_jungseong(0x1161) || !hannanum_code_is_jongseong(0x11a8)) {
    fprintf(stderr, "jamo class check failed\n");
    return 1;
  }
  if (hannanum_code_to_compatibility_jamo(0x1100) != 0x3131) {
    fprintf(stderr, "compatibility jamo check failed\n");
    return 1;
  }
  return 0;
}
