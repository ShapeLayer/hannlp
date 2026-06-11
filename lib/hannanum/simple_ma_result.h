#ifndef HANNANUM_SIMPLE_MA_RESULT_H
#define HANNANUM_SIMPLE_MA_RESULT_H

/* Port of SimpleMAResult09/22 morpheme processors. */

static int candidate_signature(const eojeol_t *eojeol, char **out);
static int candidate_list_contains_signature(candidate_list_t *list, const eojeol_t *candidate);
static int simple_ma_process_list(candidate_list_t *list, int level);

#endif
