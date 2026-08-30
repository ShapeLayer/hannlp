#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct hannanum_trie hannanum_trie_t;
typedef struct morpheme_chart morpheme_chart_t;

typedef struct hannanum {
  hannanum_trie_t *system_trie;
  hannanum_trie_t *user_trie;
  tag_group_t tag_types[HANNANUM_TAG_TYPE_COUNT];
} hannanum_t;

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

static void
free_eojeol(eojeol_t *e)
{
  size_t i;
  if (e == NULL) {
    return;
  }
  for (i = 0; i < e->length; i++) {
    free(e->morphemes[i]);
    free(e->tags[i]);
  }
  free(e->morphemes);
  free(e->tags);
  e->morphemes = NULL;
  e->tags = NULL;
  e->length = 0;
}

static int
candidate_list_add(candidate_list_t *list, eojeol_t eojeol)
{
  eojeol_t *next;
  if (list->count == list->capacity) {
    size_t new_capacity = list->capacity == 0 ? 2 : list->capacity * 2;
    next = (eojeol_t *)realloc(list->items, new_capacity * sizeof(eojeol_t));
    if (next == NULL) {
      return 0;
    }
    list->items = next;
    list->capacity = new_capacity;
  }
  list->items[list->count++] = eojeol;
  return 1;
}

static void
free_candidate_list(candidate_list_t *list)
{
  size_t i;
  for (i = 0; i < list->count; i++) {
    free_eojeol(&list->items[i]);
  }
  free(list->items);
  memset(list, 0, sizeof(*list));
}

#include "strbuffer.c"
#include "code.c"
#include "trie.c"
#include "number_dic.c"
#include "simti.c"
#include "segment_position.c"
static int
int_list_contains(const int *items, size_t count, int value)
{
  size_t i;
  for (i = 0; i < count; i++) {
    if (items[i] == value) {
      return 1;
    }
  }
  return 0;
}

static int
int_list_add_unique(int **items, size_t *count, size_t *capacity, int value)
{
  int *next;
  if (value < 0 || int_list_contains(*items, *count, value)) {
    return 1;
  }
  if (*count == *capacity) {
    size_t new_capacity = *capacity == 0 ? 8 : *capacity * 2;
    next = (int *)realloc(*items, new_capacity * sizeof(int));
    if (next == NULL) {
      return 0;
    }
    *items = next;
    *capacity = new_capacity;
  }
  (*items)[(*count)++] = value;
  return 1;
}

static int
check_tag_type(hannanum_t *h, int type, int tag)
{
  if (type == HANNANUM_TAG_TYPE_ALL) {
    return 1;
  }
  return int_list_contains(h->tag_types[type].ids, h->tag_types[type].count, tag);
}

static int
check_phoneme_type(int phoneme_type, int phoneme)
{
  return phoneme_type == 0 || phoneme_type == phoneme;
}

static int
tag_id(hannanum_t *h, const char *tag)
{
  (void)h;
  return strcmp(tag, "nnc") == 0 ? 2 : -1;
}

static int
test_expander(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int from, const char *morph_text, void *userdata)
{
  int *called = (int *)userdata;
  (void)h;
  (void)chart;
  (void)sp;
  (void)simti;
  (void)from;
  if (strcmp(morph_text, "") != 0) {
    return 0;
  }
  (*called)++;
  return 1;
}

#include "morpheme_chart.c"
#define HANNANUM_WITH_CHART_BRIDGE 1
#include "exp.c"
#undef HANNANUM_WITH_CHART_BRIDGE

static int
test_connector(hannanum_t *h, const morpheme_chart_node_t *left, const morpheme_chart_node_t *right, void *userdata)
{
  int *called = (int *)userdata;
  (void)h;
  if (left->tag == 99 && right->tag == 0) {
    (*called)++;
    return 1;
  }
  return 0;
}

static int
test_tag_type_check(hannanum_t *h, int tag_type, int tag, void *userdata)
{
  (void)h;
  (void)userdata;
  return tag_type == HANNANUM_TAG_TYPE_ALL || tag == 0;
}

static int
test_recursor(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int chart_index, int next_tag_type, void *userdata)
{
  int *called = (int *)userdata;
  (void)h;
  (void)chart;
  (void)sp;
  (void)chart_index;
  (void)next_tag_type;
  (*called)++;
  return 1;
}

static int
test_exp_prule_expander(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int from, const char *morph_text, void *userdata)
{
  exp_irregular_ids_t *ids = (exp_irregular_ids_t *)userdata;
  codepoint_vec_t prev;
  codepoint_vec_t str;
  exp_change_t changes[32];
  size_t count;
  int added;
  memset(&prev, 0, sizeof(prev));
  memset(&str, 0, sizeof(str));
  if (!hannanum_code_to_triple(morph_text, &prev) || !hannanum_code_to_triple("가", &str)) {
    codepoint_vec_free(&prev);
    codepoint_vec_free(&str);
    return 0;
  }
  count = exp_prule_generate(&prev, &str, ids, changes, 32);
  added = exp_apply_changes_to_chart(h, chart, sp, simti, from, changes, count);
  while (count > 0) {
    exp_change_free(&changes[--count]);
  }
  codepoint_vec_free(&prev);
  codepoint_vec_free(&str);
  return added;
}

int
main(void)
{
  morpheme_chart_t chart;
  int first;
  int second;
  int path[2];
  int candidates[2];
  char *tags[] = { "ncn", "jco" };
  eojeol_t eojeol;
  segment_position_t sp;
  simti_t *simti;
  codepoint_vec_t triple;
  codepoint_vec_t front;
  codepoint_vec_t back;
  codepoint_vec_t back_nd;
  codepoint_vec_t jp_front;
  hannanum_t h;
  candidate_list_t collected;
  char *collect_tags[] = { "ncn", "jco" };
  size_t tag_capacity = 0;
  int alt;
  int added;
  int exp_chart_node;
  char *replaced;
  int expander_called = 0;
  int connector_called = 0;
  int recursor_called = 0;
  exp_irregular_ids_t irr_ids = { 1, 2, 3, 4, 5, 6 };
  if (!morpheme_chart_init(&chart)) {
    return 1;
  }
  replaced = morpheme_chart_pre_replace(&chart, "ABC학교漢字");
  if (replaced == NULL || strcmp(replaced, "HAN_ENG학교HAN_CHI") != 0 || chart.eng_replacement_count != 1 || chart.chi_replacement_count != 1) {
    fprintf(stderr, "preReplace mismatch: %s\n", replaced != NULL ? replaced : "(null)");
    free(replaced);
    return 1;
  }
  free(replaced);
  first = morpheme_chart_add(&chart, 0, 0, 1, 0, "학교");
  second = morpheme_chart_add(&chart, 1, 0, 2, 0, "를");
  if (first < 0 || second < 0) {
    morpheme_chart_clear(&chart);
    return 1;
  }
  candidates[0] = first;
  candidates[1] = second;
  if (!morpheme_chart_check(&chart, candidates, 2, 0, 0, 1, 0, "학교")) {
    fprintf(stderr, "chart check failed\n");
    morpheme_chart_clear(&chart);
    return 1;
  }
  if (!morpheme_chart_add_connection(&chart, first, second)) {
    fprintf(stderr, "connection add failed\n");
    morpheme_chart_clear(&chart);
    return 1;
  }
  path[0] = first;
  path[1] = second;
  if (!morpheme_chart_node_to_eojeol(&chart, path, 2, &eojeol, tags, 2)) {
    fprintf(stderr, "eojeol conversion failed\n");
    morpheme_chart_clear(&chart);
    return 1;
  }
  if (eojeol.length != 2 || strcmp(eojeol.morphemes[0], "학교") != 0 || strcmp(eojeol.tags[1], "jco") != 0) {
    fprintf(stderr, "eojeol conversion mismatch\n");
    free_eojeol(&eojeol);
    morpheme_chart_clear(&chart);
    return 1;
  }
  free_eojeol(&eojeol);
  simti = simti_create();
  if (simti == NULL) {
    morpheme_chart_clear(&chart);
    return 1;
  }
  memset(&triple, 0, sizeof(triple));
  if (!hannanum_code_to_triple("가", &triple) || !segment_position_init_from_triple_with_simti(&sp, &triple, simti)) {
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  alt = morpheme_chart_alt_segment(&sp, simti, &triple);
  if (alt != 1) {
    fprintf(stderr, "alt segment returned %d\n", alt);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  if (!morpheme_chart_init_word(&chart, &sp, simti, "가", 99)) {
    fprintf(stderr, "chart init word failed\n");
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  if (chart.chart_end != 1 || chart.nodes[0].tag != 99 || chart.nodes[0].next_position != 1 || chart.nodes[0].state != HANNANUM_MORPHEME_STATE_SUCCESS || sp.positions[0].morph_count != 1) {
    fprintf(stderr, "chart init word mismatch\n");
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  memset(&h, 0, sizeof(h));
  h.system_trie = trie_create();
  h.user_trie = trie_create();
  if (h.system_trie == NULL || h.user_trie == NULL) {
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  if (!int_list_add_unique(&h.tag_types[HANNANUM_TAG_TYPE_YONGS].ids, &h.tag_types[HANNANUM_TAG_TYPE_YONGS].count, &tag_capacity, 0)) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  tag_capacity = 0;
  if (!int_list_add_unique(&h.tag_types[HANNANUM_TAG_TYPE_JP].ids, &h.tag_types[HANNANUM_TAG_TYPE_JP].count, &tag_capacity, 1)) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  memset(&front, 0, sizeof(front));
  memset(&back, 0, sizeof(back));
  memset(&back_nd, 0, sizeof(back_nd));
  memset(&jp_front, 0, sizeof(jp_front));
  if (!hannanum_code_to_triple("가", &front) || !hannanum_code_to_triple("아", &back)) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  if (!trie_store_codepoints(h.system_trie, front.items, front.count, 0, 0)) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  if (!hannanum_code_to_triple("이", &jp_front) || !trie_store_codepoints(h.system_trie, jp_front.items, jp_front.count, 1, 0)) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&jp_front);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    free(h.tag_types[HANNANUM_TAG_TYPE_YONGS].ids);
    free(h.tag_types[HANNANUM_TAG_TYPE_JP].ids);
    return 1;
  }
  added = morpheme_chart_phoneme_change(&h, &chart, &sp, simti, 1, &front, &back, HANNANUM_TAG_TYPE_YONGS, HANNANUM_TAG_TYPE_EOMIES, 0);
  if (added != 1) {
    fprintf(stderr, "phoneme change added %d nodes\n", added);
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    free(h.tag_types[HANNANUM_TAG_TYPE_YONGS].ids);
    return 1;
  }
  if (!hannanum_code_to_triple("ㄴ다", &back_nd)) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    free(h.tag_types[HANNANUM_TAG_TYPE_YONGS].ids);
    return 1;
  }
  if (!morpheme_chart_init_word(&chart, &sp, simti, "간다", 99)) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&back_nd);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    free(h.tag_types[HANNANUM_TAG_TYPE_YONGS].ids);
    return 1;
  }
  added = morpheme_chart_phoneme_change(&h, &chart, &sp, simti, 1, &front, &back_nd, HANNANUM_TAG_TYPE_YONGS, HANNANUM_TAG_TYPE_EOMIES, 0);
  if (added != 1) {
    fprintf(stderr, "phoneme change for back ㄴ다 added %d nodes\n", added);
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&back_nd);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    free(h.tag_types[HANNANUM_TAG_TYPE_YONGS].ids);
    return 1;
  }
  free(h.tag_types[HANNANUM_TAG_TYPE_YONGS].ids);
  h.tag_types[HANNANUM_TAG_TYPE_YONGS].ids = NULL;
  h.tag_types[HANNANUM_TAG_TYPE_YONGS].count = 0;
  if (!morpheme_chart_init_word(&chart, &sp, simti, "가", 99)) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  if (morpheme_chart_scan_dictionaries(&h, &chart, &sp, 0) < 1 || sp.positions[1].morph_count < 1 || chart.chart_end < 2) {
    fprintf(stderr, "dictionary scan failed\n");
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  if (sp.positions[1].state != HANNANUM_SP_STATE_D) {
    fprintf(stderr, "dictionary scan state mismatch\n");
    return 1;
  }
  if (morpheme_chart_expand_d_state(&h, &chart, &sp, simti, 0, test_expander, &expander_called) != 1 || expander_called != 1 || sp.positions[1].state != HANNANUM_SP_STATE_R) {
    fprintf(stderr, "D-state expansion failed\n");
    return 1;
  }
  sp.positions[1].state = HANNANUM_SP_STATE_M;
  chart.nodes[1].state = HANNANUM_MORPHEME_STATE_SUCCESS;
  if (morpheme_chart_connect_m_state(&h, &chart, &sp, 0, test_connector, &connector_called) != 1 || connector_called != 1 || chart.nodes[0].connection_count != 1) {
    fprintf(stderr, "M-state connection failed\n");
    return 1;
  }
  memset(&collected, 0, sizeof(collected));
  chart.nodes[1].connection[chart.nodes[1].connection_count++] = 0;
  if (!morpheme_chart_collect_results(&chart, 0, collect_tags, 2, &collected) || collected.count != 1 || collected.items[0].length != 1 || strcmp(collected.items[0].morphemes[0], "가") != 0) {
    fprintf(stderr, "chart result collection failed\n");
    free_candidate_list(&collected);
    return 1;
  }
  free_candidate_list(&collected);
  sp.positions[1].state = HANNANUM_SP_STATE_R;
  chart.nodes[1].state = HANNANUM_MORPHEME_STATE_INCOMPLETE;
  if (morpheme_chart_process_r_state(&h, &chart, &sp, 0, HANNANUM_TAG_TYPE_ALL, test_tag_type_check, test_recursor, &recursor_called) != 1 || recursor_called != 1 || chart.nodes[1].state != HANNANUM_MORPHEME_STATE_SUCCESS || sp.positions[1].state != HANNANUM_SP_STATE_M) {
    fprintf(stderr, "R-state processing failed\n");
    return 1;
  }
  connector_called = 0;
  chart.nodes[0].connection_count = 0;
  if (morpheme_chart_analyze_with_callbacks(&h, &chart, &sp, simti, 0, HANNANUM_TAG_TYPE_ALL, test_expander, test_tag_type_check, test_connector, &connector_called) != 1 || connector_called != 1 || chart.nodes[0].connection_count != 1) {
    fprintf(stderr, "analyze adapter failed\n");
    return 1;
  }
  if (!morpheme_chart_init_word(&chart, &sp, simti, "가", 99)) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    return 1;
  }
  morpheme_chart_scan_dictionaries(&h, &chart, &sp, 0);
  exp_chart_node = morpheme_chart_add(&chart, 0, 0, 1, 0, "아");
  if (exp_chart_node < 0) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&jp_front);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  sp.positions[1].state = HANNANUM_SP_STATE_D;
  if (morpheme_chart_expand_d_state(&h, &chart, &sp, simti, exp_chart_node, test_exp_prule_expander, &irr_ids) < 1) {
    fprintf(stderr, "Exp prule D-state expansion failed\n");
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&jp_front);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  morpheme_chart_clear(&chart);
  if (!morpheme_chart_init_word(&chart, &sp, simti, "12", 99)) {
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    return 1;
  }
  if (morpheme_chart_scan_dictionaries(&h, &chart, &sp, 0) < 1 || chart.chart_end < 2 || chart.nodes[1].tag != 2) {
    fprintf(stderr, "number scan failed\n");
    trie_destroy(h.system_trie);
    trie_destroy(h.user_trie);
    codepoint_vec_free(&front);
    codepoint_vec_free(&back);
    codepoint_vec_free(&triple);
    simti_destroy(simti);
    morpheme_chart_clear(&chart);
    return 1;
  }
  trie_destroy(h.system_trie);
  trie_destroy(h.user_trie);
  codepoint_vec_free(&front);
  codepoint_vec_free(&back);
  codepoint_vec_free(&back_nd);
  codepoint_vec_free(&jp_front);
  codepoint_vec_free(&triple);
  free(h.tag_types[HANNANUM_TAG_TYPE_JP].ids);
  simti_destroy(simti);
  morpheme_chart_clear(&chart);
  return 0;
}
