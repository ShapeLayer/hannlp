#ifndef HANNANUM_HMM_POS_TAGGER_H
#define HANNANUM_HMM_POS_TAGGER_H

/* Port of MajorPlugin/PosTagger/HmmPosTagger. */

static char phrase_char_at(const char *tag, int pos);
static int tag_starts(const eojeol_t *e, size_t i, const char *prefix);
static void phrase_tag(const eojeol_t *e, char out[3]);
static double prob_or_default(prob_entry_t **table, const char *key);
static double compute_wt(hannanum_t *h, const eojeol_t *e);
static double wp_transition(hannanum_t *h, const char *from, const char *to);
static int select_best(hannanum_t *h, candidate_list_t *sets, size_t count, size_t *selected);
static int candidate_list_is_sentence_final(const candidate_list_t *list);
static int candidate_first_is_single_sentence_final(const candidate_list_t *list);
static int select_best_by_sentence(hannanum_t *h, candidate_list_t *sets, size_t count, size_t *selected);

#endif
