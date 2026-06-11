#ifndef HANNANUM_SHARE_H
#define HANNANUM_SHARE_H

/* Shared low-level helpers corresponding to kr.ac.kaist.swrc.jhannanum.share. */

static int utf8_decode_one(const unsigned char *s, unsigned int *codepoint, size_t *width);
static int last_hangul_syllable(const char *s, unsigned int *syllable);
static int hangul_has_positive_vowel(const char *s);
static int hangul_final_is_vowel_or_l(const char *s);
static unsigned long hash_string(const char *s);
static int starts_with(const char *s, const char *prefix);
static void set_error(hannanum_t *h, const char *message);

#endif
