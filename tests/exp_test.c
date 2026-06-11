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
#include "exp.c"

static int
to_vec(const char *text, codepoint_vec_t *out)
{
  const unsigned char *p = (const unsigned char *)text;
  memset(out, 0, sizeof(*out));
  while (*p != '\0') {
    unsigned int cp;
    size_t width;
    if (!utf8_decode_one(p, &cp, &width)) {
      codepoint_vec_free(out);
      return 0;
    }
    if (!codepoint_vec_push(out, cp)) {
      codepoint_vec_free(out);
      return 0;
    }
    p += width;
  }
  return 1;
}

int
main(void)
{
  codepoint_vec_t base;
  codepoint_vec_t insert;
  codepoint_vec_t out;
  codepoint_vec_t eomi;
  codepoint_vec_t johwa;
  codepoint_vec_t shorten;
  codepoint_vec_t rem;
  codepoint_vec_t prev;
  codepoint_vec_t i_rule;
  codepoint_vec_t irr2;
  codepoint_vec_t np;
  codepoint_vec_t gut;
  codepoint_vec_t irr;
  exp_irregular_ids_t irr_ids = { 1, 2, 3, 4, 5, 6 };
  exp_change_t changes[32];
  size_t change_count;
  unsigned int cp;
  size_t width;
  if (!utf8_decode_one((const unsigned char *)"ᅡ", &cp, &width) || !exp_pcheck_cp(cp, "양성모음")) {
    fprintf(stderr, "positive vowel pcheck failed\n");
    return 1;
  }
  if (!utf8_decode_one((const unsigned char *)"ᄀ", &cp, &width) || !exp_pcheck_cp(cp, "초성")) {
    fprintf(stderr, "choseong pcheck failed\n");
    return 1;
  }
  if (!to_vec("가", &base) || !to_vec("ᄋ", &insert)) {
    return 1;
  }
  if (!exp_vec_insert(&base, 1, &insert, &out) || out.count != 3) {
    fprintf(stderr, "insert failed\n");
    codepoint_vec_free(&base);
    codepoint_vec_free(&insert);
    return 1;
  }
  codepoint_vec_free(&out);
  if (!utf8_decode_one((const unsigned char *)"ᅥ", &cp, &width) || !exp_vec_replace(&base, 1, cp, &out) || out.count != 2 || out.items[1] != cp) {
    fprintf(stderr, "replace failed\n");
    codepoint_vec_free(&base);
    codepoint_vec_free(&insert);
    return 1;
  }
  codepoint_vec_free(&out);
  codepoint_vec_free(&base);
  codepoint_vec_free(&insert);
  if (!to_vec("간다", &eomi)) {
    return 1;
  }
  change_count = exp_rule_eomi_u_generate(&eomi, 2, changes, 4);
  if (change_count != 2 || changes[0].front.count != 2 || changes[0].back.count != 5 || changes[1].front.count != 2 || changes[1].back.count != 5) {
    fprintf(stderr, "rule_eomi_u generation failed: %zu\n", change_count);
    codepoint_vec_free(&eomi);
    return 1;
  }
  exp_change_free(&changes[0]);
  exp_change_free(&changes[1]);
  codepoint_vec_free(&eomi);
  if (!to_vec("ᅡ아", &johwa)) {
    return 1;
  }
  change_count = exp_rule_johwa_generate(&johwa, 1, changes, 4);
  if (change_count != 1 || changes[0].front.count != 1 || changes[0].back.count != 2) {
    fprintf(stderr, "rule_johwa short generation failed: %zu\n", change_count);
    codepoint_vec_free(&johwa);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&johwa);
  if (!to_vec("ᅡᆫ아", &johwa)) {
    return 1;
  }
  change_count = exp_rule_johwa_generate(&johwa, 1, changes, 4);
  if (change_count != 1 || changes[0].front.count != 2 || changes[0].back.count != 2) {
    fprintf(stderr, "rule_johwa long generation failed: %zu\n", change_count);
    codepoint_vec_free(&johwa);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&johwa);
  if (!to_vec("ᅡᅪ", &shorten)) {
    return 1;
  }
  change_count = exp_rule_shorten_generate(&shorten, 1, changes, 4);
  if (change_count != 1 || changes[0].front.count != 2 || changes[0].back.count != 2) {
    fprintf(stderr, "rule_shorten o/u generation failed: %zu\n", change_count);
    codepoint_vec_free(&shorten);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&shorten);
  if (!to_vec("ᅡᅫ", &shorten)) {
    return 1;
  }
  change_count = exp_rule_shorten_generate(&shorten, 1, changes, 4);
  if (change_count != 1 || changes[0].front.count != 2 || changes[0].back.count != 2) {
    fprintf(stderr, "rule_shorten oe generation failed: %zu\n", change_count);
    codepoint_vec_free(&shorten);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&shorten);
  if (!to_vec("ᅡᅧ", &shorten)) {
    return 1;
  }
  change_count = exp_rule_shorten_generate(&shorten, 1, changes, 4);
  if (change_count != 1 || changes[0].front.count != 2 || changes[0].back.count != 2) {
    fprintf(stderr, "rule_shorten i generation failed: %zu\n", change_count);
    codepoint_vec_free(&shorten);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&shorten);
  if (!to_vec("ᅡᄂ", &rem)) {
    return 1;
  }
  change_count = exp_rule_rem_generate(&rem, 1, changes, 4);
  if (change_count < 1 || changes[0].front.count != 2) {
    fprintf(stderr, "rule_rem l-elision generation failed: %zu\n", change_count);
    codepoint_vec_free(&rem);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&rem);
  if (!to_vec("거", &rem)) {
    return 1;
  }
  change_count = exp_rule_rem_generate(&rem, 1, changes, 4);
  if (change_count < 1) {
    fprintf(stderr, "rule_rem eu/eo-elision generation failed: %zu\n", change_count);
    codepoint_vec_free(&rem);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&rem);
  if (!to_vec("가", &rem)) {
    return 1;
  }
  change_count = exp_rule_rem_generate(&rem, 1, changes, 4);
  if (change_count != 2) {
    fprintf(stderr, "rule_rem a-elision generation failed: %zu\n", change_count);
    codepoint_vec_free(&rem);
    return 1;
  }
  exp_change_free(&changes[0]);
  exp_change_free(&changes[1]);
  codepoint_vec_free(&rem);
  if (!to_vec("ᅡ", &prev) || !to_vec("여", &i_rule)) {
    return 1;
  }
  change_count = exp_rule_i_generate(&prev, &i_rule, 0, changes, 4);
  if (change_count != 1 || changes[0].front_tag_type != EXP_TAG_TYPE_JP) {
    fprintf(stderr, "rule_i yeo generation failed: %zu\n", change_count);
    codepoint_vec_free(&prev);
    codepoint_vec_free(&i_rule);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&i_rule);
  if (!to_vec("가", &i_rule)) {
    codepoint_vec_free(&prev);
    return 1;
  }
  change_count = exp_rule_i_generate(&prev, &i_rule, 0, changes, 4);
  if (change_count < 1 || changes[0].front.count != 2 || changes[0].front_tag_type != EXP_TAG_TYPE_JP) {
    fprintf(stderr, "rule_i insertion generation failed: %zu\n", change_count);
    codepoint_vec_free(&prev);
    codepoint_vec_free(&i_rule);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&prev);
  codepoint_vec_free(&i_rule);
  if (!to_vec("퍼", &irr2)) {
    return 1;
  }
  change_count = exp_rule_irr_word2_generate(&irr2, 1, changes, 4);
  if (change_count != 1 || changes[0].front.count != 2 || changes[0].back.count != 2) {
    fprintf(stderr, "rule_irr_word2 u generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr2);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&irr2);
  if (!to_vec("하여", &irr2)) {
    return 1;
  }
  change_count = exp_rule_irr_word2_generate(&irr2, 1, changes, 4);
  if (change_count < 1) {
    fprintf(stderr, "rule_irr_word2 yeo replace generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr2);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&irr2);
  if (!to_vec("해", &irr2)) {
    return 1;
  }
  change_count = exp_rule_irr_word2_generate(&irr2, 1, changes, 4);
  if (change_count != 1 || changes[0].front.count != 2 || changes[0].back.count != 2) {
    fprintf(stderr, "rule_irr_word2 yeo insert generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr2);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&irr2);
  if (!to_vec("내가", &np)) {
    return 1;
  }
  change_count = exp_rule_np_generate(&np, changes, 4);
  if (change_count != 1 || changes[0].front_tag_type != EXP_TAG_TYPE_NBNP || changes[0].back_tag_type != EXP_TAG_TYPE_JOSA) {
    fprintf(stderr, "rule_NP naega generation failed: %zu\n", change_count);
    codepoint_vec_free(&np);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&np);
  if (!to_vec("난", &np)) {
    return 1;
  }
  change_count = exp_rule_np_generate(&np, changes, 4);
  if (change_count != 1 || changes[0].back.count != 3) {
    fprintf(stderr, "rule_NP nan generation failed: %zu\n", change_count);
    codepoint_vec_free(&np);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&np);
  if (!to_vec("무언가", &np)) {
    return 1;
  }
  change_count = exp_rule_np_generate(&np, changes, 4);
  if (change_count != 1 || changes[0].front.count != 5) {
    fprintf(stderr, "rule_NP mueonga generation failed: %zu\n", change_count);
    codepoint_vec_free(&np);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&np);
  if (!to_vec("겁니", &gut)) {
    return 1;
  }
  change_count = exp_rule_gut_generate(&gut, 2, changes, 4);
  if (change_count != 1 || changes[0].front_tag_type != EXP_TAG_TYPE_NBNP || changes[0].back_tag_type != EXP_TAG_TYPE_JP) {
    fprintf(stderr, "rule_gut bnida generation failed: %zu\n", change_count);
    codepoint_vec_free(&gut);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&gut);
  if (!to_vec("걸로", &gut)) {
    return 1;
  }
  change_count = exp_rule_gut_generate(&gut, 2, changes, 4);
  if (change_count != 1 || changes[0].back_tag_type != EXP_TAG_TYPE_JOSA) {
    fprintf(stderr, "rule_gut llo generation failed: %zu\n", change_count);
    codepoint_vec_free(&gut);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&gut);
  if (!to_vec("건", &gut)) {
    return 1;
  }
  change_count = exp_rule_gut_generate(&gut, 2, changes, 4);
  if (change_count != 1 || changes[0].back_tag_type != EXP_TAG_TYPE_JOSA) {
    fprintf(stderr, "rule_gut final n generation failed: %zu\n", change_count);
    codepoint_vec_free(&gut);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&gut);
  if (!to_vec("ᆯ어", &irr)) {
    return 1;
  }
  change_count = exp_rule_irr_word_generate_basic(&irr, 1, &irr_ids, changes, 4);
  if (change_count != 1 || changes[0].phoneme != irr_ids.type_d) {
    fprintf(stderr, "rule_irr_word d generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&irr);
  if (!to_vec("ᅡ어", &irr)) {
    return 1;
  }
  change_count = exp_rule_irr_word_generate_basic(&irr, 1, &irr_ids, changes, 4);
  if (change_count < 1 || changes[0].phoneme != irr_ids.type_s) {
    fprintf(stderr, "rule_irr_word s generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&irr);
  if (!to_vec("우", &irr)) {
    return 1;
  }
  change_count = exp_rule_irr_word_generate_basic(&irr, 1, &irr_ids, changes, 4);
  if (change_count != 1 || changes[0].phoneme != irr_ids.type_b) {
    fprintf(stderr, "rule_irr_word b generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr);
    return 1;
  }
  exp_change_free(&changes[0]);
  codepoint_vec_free(&irr);
  if (!to_vec("가ᅡ", &irr)) {
    return 1;
  }
  change_count = exp_rule_irr_word_generate_basic(&irr, 1, &irr_ids, changes, 4);
  if (change_count < 1 || changes[0].phoneme != irr_ids.type_h) {
    fprintf(stderr, "rule_irr_word h26 generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&irr);
  if (!to_vec("개ᄋ", &irr)) {
    return 1;
  }
  change_count = exp_rule_irr_word_generate_basic(&irr, 1, &irr_ids, changes, 4);
  if (change_count < 2 || changes[0].phoneme != irr_ids.type_h || changes[1].phoneme != irr_ids.type_h) {
    fprintf(stderr, "rule_irr_word h27 generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&irr);
  if (!to_vec("거ᄋ", &irr)) {
    return 1;
  }
  change_count = exp_rule_irr_word_generate_basic(&irr, 1, &irr_ids, changes, 4);
  if (change_count < 1 || changes[0].phoneme != irr_ids.type_h) {
    fprintf(stderr, "rule_irr_word h28 generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&irr);
  if (!to_vec("ᆯ라", &irr)) {
    return 1;
  }
  change_count = exp_rule_irr_word_generate_basic(&irr, 1, &irr_ids, changes, 4);
  if (change_count < 1 || changes[0].phoneme != irr_ids.type_reu) {
    fprintf(stderr, "rule_irr_word reu generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&irr);
  if (!to_vec("르러", &irr)) {
    return 1;
  }
  change_count = exp_rule_irr_word_generate_basic(&irr, 2, &irr_ids, changes, 4);
  if (change_count < 1 || changes[0].phoneme != irr_ids.type_reo) {
    fprintf(stderr, "rule_irr_word reo generation failed: %zu\n", change_count);
    codepoint_vec_free(&irr);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&irr);
  if (!to_vec("ᅡ", &prev) || !to_vec("가", &i_rule)) {
    return 1;
  }
  change_count = exp_prule_generate(&prev, &i_rule, &irr_ids, changes, 32);
  if (change_count < 1) {
    fprintf(stderr, "exp_prule generation failed: %zu\n", change_count);
    codepoint_vec_free(&prev);
    codepoint_vec_free(&i_rule);
    return 1;
  }
  {
    size_t k;
    int saw_ga = 0;
    char *front_text;
    for (k = 0; k < change_count; k++) {
      front_text = hannanum_code_from_triple(&changes[k].front);
      if (front_text != NULL && strcmp(front_text, "가") == 0) {
        saw_ga = 1;
      }
      free(front_text);
    }
    if (!saw_ga) {
      fprintf(stderr, "exp_prule gan-da missing expected generic stem: ga=%d\n", saw_ga);
      while (change_count > 0) {
        exp_change_free(&changes[--change_count]);
      }
      codepoint_vec_free(&prev);
      codepoint_vec_free(&i_rule);
      return 1;
    }
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&prev);
  codepoint_vec_free(&i_rule);
  if (!to_vec("", &prev) || !to_vec("간다", &i_rule)) {
    return 1;
  }
  change_count = exp_prule_generate(&prev, &i_rule, &irr_ids, changes, 32);
  if (change_count < 1) {
    fprintf(stderr, "exp_prule gan-da generation failed: %zu\n", change_count);
    codepoint_vec_free(&prev);
    codepoint_vec_free(&i_rule);
    return 1;
  }
  while (change_count > 0) {
    exp_change_free(&changes[--change_count]);
  }
  codepoint_vec_free(&prev);
  codepoint_vec_free(&i_rule);
  return 0;
}
