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
#include "simti.c"
#include "segment_position.c"

int
main(void)
{
  segment_position_t sp;
  simti_t *simti;
  codepoint_vec_t triple;
  unsigned int reverse_word[2];
  int index;
  if (!segment_position_init_text(&sp, "가")) {
    fprintf(stderr, "failed to init segment position\n");
    return 1;
  }
  if (sp.position_end != 3) {
    fprintf(stderr, "unexpected position count: %d\n", sp.position_end);
    return 1;
  }
  if (sp.positions[0].key != HANNANUM_POSITION_START_KEY || sp.positions[0].state != HANNANUM_SP_STATE_M) {
    fprintf(stderr, "start position mismatch\n");
    return 1;
  }
  if (segment_position_next(&sp, 0) != 1 || segment_position_next(&sp, 1) != 2 || segment_position_next(&sp, 2) != 0) {
    fprintf(stderr, "position links mismatch\n");
    return 1;
  }
  index = segment_position_add(&sp, 0x1100);
  if (index <= 0) {
    fprintf(stderr, "add position failed\n");
    return 1;
  }
  if (segment_position_get(&sp, index)->key != 0x1100) {
    fprintf(stderr, "get position failed\n");
    return 1;
  }
  simti = simti_create();
  if (simti == NULL) {
    return 1;
  }
  memset(&triple, 0, sizeof(triple));
  if (!hannanum_code_to_triple("가", &triple)) {
    simti_destroy(simti);
    return 1;
  }
  if (!segment_position_init_from_triple_with_simti(&sp, &triple, simti)) {
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    return 1;
  }
  reverse_word[0] = triple.items[1];
  reverse_word[1] = triple.items[0];
  if (simti_fetch(simti, reverse_word, 2) != 1) {
    fprintf(stderr, "simti reverse fetch failed\n");
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    return 1;
  }
  codepoint_vec_free(&triple);
  simti_destroy(simti);
  return 0;
}
