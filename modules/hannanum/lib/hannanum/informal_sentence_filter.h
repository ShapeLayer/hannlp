// derived from SupplementPlugin/PlainTextProcessor/InformalSentenceFilter

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


#ifndef HANNANUM_INFORMAL_SENTENCE_FILTER_H
#define HANNANUM_INFORMAL_SENTENCE_FILTER_H

static int utf8_char_width_from_first(unsigned char c);
static size_t utf8_char_count(const char *s, size_t len);
static int filter_informal_word(const char *word, size_t len, char **buffer, size_t *used, size_t *capacity);
static char *informal_sentence_filter(const char *input);

#endif
