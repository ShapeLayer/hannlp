// derived from kr.ac.kaist.swrc.jhannanum.share

/*  Copyright 2010, 2011 Semantic Web Research Center, KAIST

This file is part of JHanNanum.

JHanNanum is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

JHanNanum is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with JHanNanum.  If not, see <http://www.gnu.org/licenses/>   */


#ifndef HANNANUM_SHARE_H
#define HANNANUM_SHARE_H

static int utf8_decode_one(const unsigned char *s, unsigned int *codepoint, size_t *width);
static int last_hangul_syllable(const char *s, unsigned int *syllable);
static int hangul_has_positive_vowel(const char *s);
static int hangul_final_is_vowel_or_l(const char *s);
static unsigned long hash_string(const char *s);
static int starts_with(const char *s, const char *prefix);
static void set_error(hannanum_t *h, const char *message);

#endif
