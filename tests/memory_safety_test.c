#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Fail exactly one allocation after the fixture has been constructed. */
static int malloc_countdown;
static int realloc_countdown;
static void *test_malloc(size_t size)
{
  if (malloc_countdown > 0 && --malloc_countdown == 0) return NULL;
  return malloc(size);
}
static void *test_realloc(void *ptr, size_t size)
{
  if (realloc_countdown > 0 && --realloc_countdown == 0) return NULL;
  return realloc(ptr, size);
}
#define malloc test_malloc
#define realloc test_realloc
#include "hannanum.c"
#undef malloc
#undef realloc

#define CHECK(expr) do { if (!(expr)) { \
  fprintf(stderr, "Failed at line %d: %s\n", __LINE__, #expr); exit(1); \
} } while (0)

static void test_candidate_ownership(void)
{
  candidate_list_t list = {0};
  eojeol_t e = make_pair("가", "pvg", "어", "ecs");
  CHECK(e.length == 2);
  malloc_countdown = 1; /* Replacement allocation in postprocess_eojeol. */
  CHECK(!candidate_list_add(&list, e));
  CHECK(list.count == 0);
  CHECK(strcmp(e.morphemes[0], "가") == 0);
  free_eojeol(&e); /* The caller still owns the failed candidate. */

  e = make_single("word", "ncn");
  realloc_countdown = 1; /* List growth also leaves ownership with caller. */
  CHECK(!candidate_list_add(&list, e));
  free_eojeol(&e);
  free_candidate_list(&list);
}

static void test_signature_failure(void)
{
  candidate_list_t list = {0};
  eojeol_t e = make_single("word", "ncn");
  CHECK(candidate_list_add(&list, clone_eojeol(&e)));
  CHECK(candidate_list_contains_signature(&list, &e) == 1);
  realloc_countdown = 1; /* Candidate signature. */
  CHECK(candidate_list_contains_signature(&list, &e) == -1);
  realloc_countdown = 2; /* Existing list entry's signature. */
  CHECK(candidate_list_contains_signature(&list, &e) == -1);
  realloc_countdown = 2; /* Tag conversion succeeds, signature fails. */
  CHECK(!simple_ma_process_list(&list, 2));
  CHECK(list.count == 1);
  CHECK(strcmp(list.items[0].tags[0], "ncn") == 0);
  CHECK(simple_ma_process_list(&list, 2));
  CHECK(strcmp(list.items[0].tags[0], "NC") == 0);
  free_eojeol(&e);
  free_candidate_list(&list);
}

static void test_result_accessors(void)
{
  hannanum_result_t *r = calloc(1, sizeof(*r));
  CHECK(r != NULL);
  r->count = 2;
  r->candidate_sets = calloc(r->count, sizeof(*r->candidate_sets));
  CHECK(r->candidate_sets != NULL);
  CHECK(candidate_list_add(&r->candidate_sets[1], make_single("word", "ncn")));
  CHECK(hannanum_result_candidate_count(r, 1) == 1);
  CHECK(hannanum_result_morpheme_count(r, 1) == 0);
  CHECK(hannanum_result_morpheme(r, 1, 0) == NULL);
  CHECK(hannanum_result_tag(r, 1, 0) == NULL);
  CHECK(hannanum_result_morpheme_count(NULL, 0) == 0);
  CHECK(hannanum_result_morpheme(NULL, 0, 0) == NULL);
  CHECK(hannanum_result_tag(NULL, 0, 0) == NULL);
  hannanum_result_destroy(r);

  r = calloc(1, sizeof(*r));
  CHECK(r != NULL);
  r->count = 1;
  r->eojeols = calloc(1, sizeof(*r->eojeols));
  CHECK(r->eojeols != NULL);
  r->eojeols[0] = make_single("word", "ncn");
  CHECK(hannanum_result_morpheme_count(r, 0) == 1);
  CHECK(strcmp(hannanum_result_morpheme(r, 0, 0), "word") == 0);
  CHECK(strcmp(hannanum_result_tag(r, 0, 0), "ncn") == 0);
  CHECK(hannanum_result_morpheme_count(r, 1) == 0);
  CHECK(hannanum_result_morpheme(r, 0, 1) == NULL);
  CHECK(hannanum_result_tag(r, 1, 0) == NULL);
  hannanum_result_destroy(r);
}

static void test_deep_trie_free(void)
{
  /* Freeing must not recurse per code point (HAN-03): a 200k-deep chain
     would overflow a small thread stack with a recursive walk. */
  hannanum_trie_t *trie = trie_create();
  trie_node_t *node;
  unsigned int *word = (unsigned int *)calloc(HANNANUM_MAX_TRIE_WORD + 1, sizeof(unsigned int));
  size_t i;
  CHECK(trie != NULL && word != NULL);
  node = &trie->root;
  for (i = 0; i < 200000; i++) {
    node = trie_node_insert_child(node, 'a' + (unsigned int)(i % 26));
    CHECK(node != NULL);
  }
  for (i = 0; i <= HANNANUM_MAX_TRIE_WORD; i++) {
    word[i] = 'a';
  }
  /* Words longer than a segment can hold are rejected at load time. */
  CHECK(!trie_store_codepoints(trie, word, HANNANUM_MAX_TRIE_WORD + 1, 1, 0));
  CHECK(trie_store_codepoints(trie, word, HANNANUM_MAX_TRIE_WORD, 1, 0));
  trie_destroy(trie);
  free(word);
}

static void test_select_best_rejects_empty_column(void)
{
  /* An empty candidate column must fail instead of back-tracking through a
     zero-sized node array (HAN-04). */
  hannanum_t h;
  candidate_list_t sets[2];
  size_t selected[2];
  memset(&h, 0, sizeof(h));
  memset(sets, 0, sizeof(sets));
  CHECK(candidate_list_add(&sets[0], make_single("word", "ncn")));
  CHECK(!select_best(&h, sets, 2, selected));
  free_candidate_list(&sets[0]);
}

static int interrupt_after_first(void *userdata)
{
  return ++*(int *)userdata > 1;
}

static void test_chart_limits_and_interrupt(const char *data_dir)
{
  hannanum_options_t options = { data_dir, HANNANUM_OUTPUT_MORPH };
  hannanum_t *h = hannanum_create(&options);
  hannanum_result_t *r;
  char word[16 * 60 + 1];
  int calls = 0;
  size_t i;
  CHECK(h != NULL);
  /* Chart paths grow exponentially with eojeol length (HAN-02). */
  for (i = 0; i < 60; i++) {
    memcpy(word + i * 12, "사과나무", 12);
  }
  word[60 * 12] = '\0';
  r = hannanum_analyze(h, word);
  CHECK(r != NULL);
  CHECK(hannanum_result_candidate_count(r, 0) <= HANNANUM_MAX_EOJEOL_CANDIDATES * 2);
  hannanum_result_destroy(r);
  /* The last eojeol is chosen by Viterbi score, not fixed to candidate 0. */
  hannanum_destroy(h);
  options.output_mode = HANNANUM_OUTPUT_HMM_POS;
  h = hannanum_create(&options);
  CHECK(h != NULL);
  r = hannanum_analyze(h, "아이들이 공원에서 놀고 있다");
  CHECK(r != NULL && hannanum_result_eojeol_count(r) == 4);
  CHECK(strcmp(hannanum_result_tag(r, 3, 0), "px") == 0);
  hannanum_result_destroy(r);
  hannanum_set_interrupt(h, interrupt_after_first, &calls);
  CHECK(hannanum_analyze(h, "하나 둘 셋") == NULL);
  CHECK(strcmp(hannanum_error(h), "analysis interrupted") == 0);
  hannanum_destroy(h);
}

static void test_strict_utf8(void)
{
  unsigned int cp;
  size_t width;
  CHECK(!utf8_decode_one((const unsigned char *)"\xC0\x80", &cp, &width));
  CHECK(!utf8_decode_one((const unsigned char *)"\xED\xA0\x80", &cp, &width));
  CHECK(!utf8_decode_one((const unsigned char *)"\xF4\x90\x80\x80", &cp, &width));
  CHECK(!utf8_decode_one((const unsigned char *)"\xE0\x80\x80", &cp, &width));
  CHECK(utf8_decode_one((const unsigned char *)"\xF4\x8F\xBF\xBF", &cp, &width) && cp == 0x10ffff);
  CHECK(utf8_decode_one((const unsigned char *)"한", &cp, &width) && cp == 0xd55c && width == 3);
}

static void test_probability_rejects_non_finite(void)
{
  /* strtod() accepts "nan"/"inf"; such values must not enter the tables. */
  char dir[] = "/tmp/hannanum-prob-XXXXXX";
  char path[sizeof(dir) + 32];
  hannanum_t h;
  prob_entry_t **table;
  FILE *fp;
  double value;
  size_t i;
  CHECK(mkdtemp(dir) != NULL);
  snprintf(path, sizeof(path), "%s/stat", dir);
  CHECK(mkdir(path, 0700) == 0);
  snprintf(path, sizeof(path), "%s/stat/T.pos", dir);
  fp = fopen(path, "w");
  CHECK(fp != NULL);
  fputs("a nan\nb inf\nc -inf\nd -1.5\n", fp);
  fclose(fp);
  memset(&h, 0, sizeof(h));
  h.data_dir = dir;
  table = (prob_entry_t **)calloc(HANNANUM_HASH_SIZE, sizeof(prob_entry_t *));
  CHECK(table != NULL);
  load_probability(&h, table, "stat/T.pos");
  CHECK(!prob_get(table, "a", &value));
  CHECK(!prob_get(table, "b", &value));
  CHECK(!prob_get(table, "c", &value));
  CHECK(prob_get(table, "d", &value) && value == -1.5);
  for (i = 0; i < HANNANUM_HASH_SIZE; i++) {
    while (table[i] != NULL) {
      prob_entry_t *next = table[i]->next;
      free(table[i]->key);
      free(table[i]);
      table[i] = next;
    }
  }
  free(table);
  remove(path);
  snprintf(path, sizeof(path), "%s/stat", dir);
  rmdir(path);
  rmdir(dir);
}

int main(int argc, char **argv)
{
  test_probability_rejects_non_finite();
  test_candidate_ownership();
  test_signature_failure();
  test_result_accessors();
  test_deep_trie_free();
  test_select_best_rejects_empty_column();
  test_strict_utf8();
  if (argc > 1) {
    test_chart_limits_and_interrupt(argv[1]);
  }
  puts("Memory safety regression tests passed.");
  return 0;
}
