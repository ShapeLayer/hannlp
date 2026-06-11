#ifndef KO_NLP_HANNANUM_H
#define KO_NLP_HANNANUM_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hannanum hannanum_t;
typedef struct hannanum_result hannanum_result_t;

typedef enum hannanum_output_mode {
  HANNANUM_OUTPUT_HMM_POS = 0,
  HANNANUM_OUTPUT_SIMPLE_POS_09 = 1,
  HANNANUM_OUTPUT_SIMPLE_POS_22 = 2,
  HANNANUM_OUTPUT_NOUNS = 3,
  HANNANUM_OUTPUT_MORPH = 4,
  HANNANUM_OUTPUT_MORPH_SIMPLE_09 = 5,
  HANNANUM_OUTPUT_MORPH_SIMPLE_22 = 6
} hannanum_output_mode_t;

typedef struct hannanum_options {
  const char *data_dir;
  hannanum_output_mode_t output_mode;
} hannanum_options_t;

hannanum_t *hannanum_create(const hannanum_options_t *options);
void hannanum_destroy(hannanum_t *hannanum);

const char *hannanum_error(const hannanum_t *hannanum);

hannanum_result_t *hannanum_analyze(hannanum_t *hannanum, const char *input);
void hannanum_result_destroy(hannanum_result_t *result);

size_t hannanum_result_eojeol_count(const hannanum_result_t *result);
size_t hannanum_result_morpheme_count(const hannanum_result_t *result, size_t eojeol_index);
const char *hannanum_result_plain(const hannanum_result_t *result, size_t eojeol_index);
const char *hannanum_result_morpheme(const hannanum_result_t *result, size_t eojeol_index, size_t morpheme_index);
const char *hannanum_result_tag(const hannanum_result_t *result, size_t eojeol_index, size_t morpheme_index);

size_t hannanum_result_candidate_count(const hannanum_result_t *result, size_t eojeol_index);
size_t hannanum_result_candidate_morpheme_count(const hannanum_result_t *result, size_t eojeol_index, size_t candidate_index);
const char *hannanum_result_candidate_morpheme(const hannanum_result_t *result, size_t eojeol_index, size_t candidate_index, size_t morpheme_index);
const char *hannanum_result_candidate_tag(const hannanum_result_t *result, size_t eojeol_index, size_t candidate_index, size_t morpheme_index);

char *hannanum_result_format(const hannanum_result_t *result);

#ifdef __cplusplus
}
#endif

#endif
