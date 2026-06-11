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
  return 0;
}

#include "strbuf.c"
#include "code.c"
#include "trie.c"

int
main(void)
{
  hannanum_trie_t *trie = trie_create();
  trie_node_t *node;
  if (trie == NULL) {
    return 1;
  }
  if (!trie_store_text(trie, "학교", 10, 0)) {
    trie_destroy(trie);
    return 1;
  }
  if (!trie_store_text(trie, "학생", 11, 2)) {
    trie_destroy(trie);
    return 1;
  }
  node = trie_fetch_text(trie, "학교");
  if (node == NULL || node->info == NULL || node->info->tag != 10) {
    fprintf(stderr, "failed to fetch 학교\n");
    trie_destroy(trie);
    return 1;
  }
  node = trie_fetch_text(trie, "학");
  if (node != NULL) {
    fprintf(stderr, "prefix unexpectedly fetched\n");
    trie_destroy(trie);
    return 1;
  }
  node = trie_fetch_text(trie, "학생");
  if (node == NULL || node->info == NULL || node->info->tag != 11 || node->info->phoneme != 2) {
    fprintf(stderr, "failed to fetch 학생\n");
    trie_destroy(trie);
    return 1;
  }
  trie_destroy(trie);
  return 0;
}
