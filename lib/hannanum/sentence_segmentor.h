#ifndef HANNANUM_SENTENCE_SEGMENTOR_H
#define HANNANUM_SENTENCE_SEGMENTOR_H

/* Port of SupplementPlugin/PlainTextProcessor/SentenceSegmentor. */

static int is_sentence_symbol(unsigned char c);
static int find_sentence_segmentor_split(const char *token, size_t len, size_t *prefix_len, size_t *symbol_len);
static int split_eojeols(const char *input, str_vec_t *out);

#endif
