// derived from SupplementPlugin/PlainTextProcessor/SentenceSegmentor

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


#ifndef HANNANUM_SENTENCE_SEGMENTOR_H
#define HANNANUM_SENTENCE_SEGMENTOR_H

static int is_sentence_symbol(unsigned char c);
static int find_sentence_segmentor_split(const char *token, size_t len, size_t *prefix_len, size_t *symbol_len);
static int split_eojeols(const char *input, str_vec_t *out);

#endif
