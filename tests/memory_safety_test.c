#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int main(void)
{
  test_candidate_ownership();
  test_signature_failure();
  test_result_accessors();
  puts("Memory safety regression tests passed.");
  return 0;
}
