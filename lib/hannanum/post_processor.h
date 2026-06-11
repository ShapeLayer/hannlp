#ifndef HANNANUM_POST_PROCESSOR_H
#define HANNANUM_POST_PROCESSOR_H

/* Port of ChartMorphAnalyzer/PostProcessor.java post-processing rules. */

static int replace_morpheme(eojeol_t *e, size_t index, const char *morpheme);
static char *drop_initial_eu_from_morpheme(const char *morpheme);
static int can_drop_initial_eu(const char *morpheme);
static int postprocess_eojeol(eojeol_t *e);

#endif
