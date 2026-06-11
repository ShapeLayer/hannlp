#ifndef HANNANUM_INFORMAL_SENTENCE_FILTER_H
#define HANNANUM_INFORMAL_SENTENCE_FILTER_H

/* Port of SupplementPlugin/PlainTextProcessor/InformalSentenceFilter. */

static int utf8_char_width_from_first(unsigned char c);
static size_t utf8_char_count(const char *s, size_t len);
static int filter_informal_word(const char *word, size_t len, char **buffer, size_t *used, size_t *capacity);
static char *informal_sentence_filter(const char *input);

#endif
