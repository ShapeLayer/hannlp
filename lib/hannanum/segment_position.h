#ifndef HANNANUM_SEGMENT_POSITION_H
#define HANNANUM_SEGMENT_POSITION_H

/* Port of ChartMorphAnalyzer/SegmentPosition.java segment position graph. */

#define HANNANUM_MAX_SEGMENT 1024
#define HANNANUM_MAX_MORPHEME_COUNT 512
#define HANNANUM_SP_STATE_N 0
#define HANNANUM_SP_STATE_D 1
#define HANNANUM_SP_STATE_R 2
#define HANNANUM_SP_STATE_M 3
#define HANNANUM_SP_STATE_F 4
#define HANNANUM_POSITION_START_KEY 0u

typedef struct segment_position_node {
  unsigned int key;
  int state;
  int next_position;
  int s_index;
  int u_index;
  int n_index;
  int morph_count;
  int morpheme[HANNANUM_MAX_MORPHEME_COUNT];
} segment_position_node_t;

typedef struct segment_position {
  segment_position_node_t positions[HANNANUM_MAX_SEGMENT];
  int position_end;
} segment_position_t;

static void segment_position_clear(segment_position_t *sp);
static int segment_position_add(segment_position_t *sp, unsigned int key);
static segment_position_node_t *segment_position_get(segment_position_t *sp, int index);
static int segment_position_next(const segment_position_t *sp, int index);
static int segment_position_set_link(segment_position_t *sp, int previous, int next);
static int segment_position_init_from_triple(segment_position_t *sp, const codepoint_vec_t *triple);
static int segment_position_init_from_triple_with_simti(segment_position_t *sp, const codepoint_vec_t *triple, simti_t *simti);
static int segment_position_init_text(segment_position_t *sp, const char *text);

#endif
