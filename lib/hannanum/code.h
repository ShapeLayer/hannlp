// derived from share/Code.java

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


#ifndef HANNANUM_CODE_H
#define HANNANUM_CODE_H

#define HANNANUM_FILLER 0x3164u

typedef struct codepoint_vec {
  unsigned int *items;
  size_t count;
  size_t capacity;
} codepoint_vec_t;

static int codepoint_vec_push(codepoint_vec_t *vec, unsigned int cp);
static void codepoint_vec_free(codepoint_vec_t *vec);
static int hannanum_code_is_choseong(unsigned int c);
static int hannanum_code_is_jungseong(unsigned int c);
static int hannanum_code_is_jongseong(unsigned int c);
static unsigned int hannanum_code_to_compatibility_jamo(unsigned int jamo);
static int hannanum_code_to_triple(const char *input, codepoint_vec_t *out);
static char *hannanum_code_from_triple(const codepoint_vec_t *triple);

#endif
