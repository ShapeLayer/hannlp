#ifndef HANNANUM_WORKFLOW_H
#define HANNANUM_WORKFLOW_H

/* Port of Workflow/WorkflowFactory orchestration exposed through hannanum.h. */

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
char *hannanum_result_format(const hannanum_result_t *result);

#endif
