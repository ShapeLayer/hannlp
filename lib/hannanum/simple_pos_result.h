#ifndef HANNANUM_SIMPLE_POS_RESULT_H
#define HANNANUM_SIMPLE_POS_RESULT_H

/* Port of SimplePOSResult09/22 and TagMapper level mapping used by POS processors. */

static char *tag_on_level(const char *tag, int level);
static int simple_pos_process_eojeol(eojeol_t *eojeol, int level);
static int simple_pos_process_result(hannanum_result_t *result, int level);

#endif
