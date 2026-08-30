#include "hannanum.h"

#include <ctype.h>
#include <errno.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strbuffer.h"

#define HANNANUM_HASH_SIZE 262144u
#define HANNANUM_DEFAULT_PROB (-20.0)
#define HANNANUM_WP_SMOOTHING (-4.605170185988091)
#define HANNANUM_MAX_SEGMENT_CANDIDATES 64u
#define HANNANUM_MAX_SEGMENT_DEPTH 8u
#if defined(__GNUC__) || defined(__clang__)
#define HANNANUM_UNUSED __attribute__((unused))
#else
#define HANNANUM_UNUSED
#endif

typedef struct {
  char **morphemes;
  char **tags;
  size_t length;
} eojeol_t;

typedef struct {
  eojeol_t *items;
  size_t count;
  size_t capacity;
} candidate_list_t;

typedef struct dict_entry {
  char *key;
  candidate_list_t candidates;
  size_t analyzed_count;
  struct dict_entry *next;
} dict_entry_t;

typedef struct prob_entry {
  char *key;
  double value;
  struct prob_entry *next;
} prob_entry_t;

typedef struct {
  char *name;
  int *ids;
  size_t count;
} tag_group_t;

enum {
  HANNANUM_TAG_TYPE_ALL = 0,
  HANNANUM_TAG_TYPE_VERBS = 1,
  HANNANUM_TAG_TYPE_NOUNS = 2,
  HANNANUM_TAG_TYPE_NPS = 3,
  HANNANUM_TAG_TYPE_ADJS = 4,
  HANNANUM_TAG_TYPE_NBNP = 5,
  HANNANUM_TAG_TYPE_JOSA = 6,
  HANNANUM_TAG_TYPE_YONGS = 7,
  HANNANUM_TAG_TYPE_EOMIES = 8,
  HANNANUM_TAG_TYPE_JP = 9,
  HANNANUM_TAG_TYPE_COUNT = 10
};

struct hannanum {
  char *data_dir;
  hannanum_output_mode_t output_mode;
  char error[256];
  dict_entry_t **dict;
  prob_entry_t **pwt;
  prob_entry_t **ptt_pos;
  prob_entry_t **ptt_wp;
  char **tag_names;
  size_t tag_count;
  char **irregular_names;
  size_t irregular_count;
  tag_group_t *tag_groups;
  size_t tag_group_count;
  tag_group_t tag_types[HANNANUM_TAG_TYPE_COUNT];
  unsigned char *connections;
  struct hannanum_trie *system_trie;
  struct hannanum_trie *user_trie;
};

struct hannanum_result {
  char **plain;
  eojeol_t *eojeols;
  candidate_list_t *candidate_sets;
  size_t count;
};

typedef struct {
  char **items;
  size_t count;
  size_t capacity;
} str_vec_t;

typedef struct {
  const eojeol_t *eojeol;
  char phrase[3];
  double wt;
  double score;
  size_t back;
  int has_back;
} hmm_node_t;

typedef struct {
  char **morphemes;
  char **tags;
  size_t count;
  size_t capacity;
} segment_stack_t;

static void free_eojeol(eojeol_t *e);

#include "strbuffer.c"
#include "share.h"
#include "share.c"
#include "code.h"
#include "code.c"
#include "post_processor.h"
#include "post_processor.c"
#include "comm.h"
#include "comm.c"
#include "tag_set.h"
#include "tag_set.c"
#include "connection.h"
#include "connection.c"
#include "connection_not.h"
#include "connection_not.c"
#include "number_dic.h"
#include "number_dic.c"
#include "trie.h"
#include "trie.c"
#include "simti.h"
#include "simti.c"
#include "segment_position.h"
#include "segment_position.c"
#include "morpheme_chart.h"
#include "morpheme_chart.c"
#include "exp.h"
#define HANNANUM_WITH_CHART_BRIDGE 1
#include "exp.c"
#undef HANNANUM_WITH_CHART_BRIDGE
#include "chart_morph_analyzer.h"
#include "chart_morph_analyzer.c"
#include "sentence_segmentor.h"
#include "sentence_segmentor.c"
#include "informal_sentence_filter.h"
#include "informal_sentence_filter.c"
#include "unknown_processor.h"
#include "unknown_processor.c"
#include "hmm_pos_tagger.h"
#include "hmm_pos_tagger.c"
#include "simple_pos_result.h"
#include "simple_pos_result.c"
#include "simple_ma_result.h"
#include "simple_ma_result.c"
#include "noun_extractor.h"
#include "noun_extractor.c"
#include "workflow.h"
#include "workflow.c"
