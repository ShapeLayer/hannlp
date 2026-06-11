#include "segment_position.h"

static void HANNANUM_UNUSED
segment_position_clear(segment_position_t *sp)
{
  if (sp == NULL) {
    return;
  }
  memset(sp, 0, sizeof(*sp));
}

static int HANNANUM_UNUSED
segment_position_add(segment_position_t *sp, unsigned int key)
{
  segment_position_node_t *position;
  if (sp == NULL || sp->position_end >= HANNANUM_MAX_SEGMENT) {
    return 0;
  }
  position = &sp->positions[sp->position_end];
  position->key = key;
  position->state = HANNANUM_SP_STATE_N;
  position->morph_count = 0;
  position->next_position = 0;
  position->s_index = 0;
  position->u_index = 0;
  position->n_index = 0;
  return sp->position_end++;
}

static segment_position_node_t HANNANUM_UNUSED *
segment_position_get(segment_position_t *sp, int index)
{
  if (sp == NULL || index < 0 || index >= sp->position_end) {
    return NULL;
  }
  return &sp->positions[index];
}

static int HANNANUM_UNUSED
segment_position_next(const segment_position_t *sp, int index)
{
  if (sp == NULL || index < 0 || index >= sp->position_end) {
    return 0;
  }
  return sp->positions[index].next_position;
}

static int HANNANUM_UNUSED
segment_position_set_link(segment_position_t *sp, int previous, int next)
{
  if (sp == NULL || previous < 0 || previous >= sp->position_end || next < 0 || next >= HANNANUM_MAX_SEGMENT) {
    return 0;
  }
  sp->positions[previous].next_position = next;
  return previous;
}

static int HANNANUM_UNUSED
segment_position_init_from_triple(segment_position_t *sp, const codepoint_vec_t *triple)
{
  int previous;
  size_t i;
  if (sp == NULL || triple == NULL) {
    return 0;
  }
  segment_position_clear(sp);
  previous = segment_position_add(sp, HANNANUM_POSITION_START_KEY);
  if (previous != 0) {
    return 0;
  }
  sp->positions[previous].state = HANNANUM_SP_STATE_M;
  for (i = 0; i < triple->count; i++) {
    int next = segment_position_add(sp, triple->items[i]);
    if (next == 0) {
      return 0;
    }
    segment_position_set_link(sp, previous, next);
    previous = next;
  }
  segment_position_set_link(sp, previous, 0);
  return 1;
}

static int HANNANUM_UNUSED
segment_position_init_from_triple_with_simti(segment_position_t *sp, const codepoint_vec_t *triple, simti_t *simti)
{
  int previous;
  size_t i;
  if (sp == NULL || triple == NULL || simti == NULL) {
    return 0;
  }
  segment_position_clear(sp);
  simti_init(simti);
  previous = segment_position_add(sp, HANNANUM_POSITION_START_KEY);
  if (previous != 0) {
    return 0;
  }
  sp->positions[previous].state = HANNANUM_SP_STATE_M;
  for (i = 0; i < triple->count; i++) {
    int next = segment_position_add(sp, triple->items[i]);
    size_t reverse_len = triple->count - i;
    unsigned int reverse_word[1024];
    size_t j;
    if (next == 0 || reverse_len > 1024) {
      return 0;
    }
    segment_position_set_link(sp, previous, next);
    for (j = 0; j < reverse_len; j++) {
      reverse_word[j] = triple->items[triple->count - 1 - j];
    }
    simti_insert(simti, reverse_word, reverse_len, next);
    previous = next;
  }
  segment_position_set_link(sp, previous, 0);
  return 1;
}

static int HANNANUM_UNUSED
segment_position_init_text(segment_position_t *sp, const char *text)
{
  codepoint_vec_t triple;
  int ok;
  memset(&triple, 0, sizeof(triple));
  if (!hannanum_code_to_triple(text, &triple)) {
    codepoint_vec_free(&triple);
    return 0;
  }
  ok = segment_position_init_from_triple(sp, &triple);
  codepoint_vec_free(&triple);
  return ok;
}
