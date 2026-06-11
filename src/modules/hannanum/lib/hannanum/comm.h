// Derived from comm.*

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


#ifndef HANNANUM_COMM_H
#define HANNANUM_COMM_H

static void free_eojeol(eojeol_t *e);
static eojeol_t clone_eojeol(const eojeol_t *src);
static int candidate_list_add(candidate_list_t *list, eojeol_t eojeol);
static int candidate_list_has_signature(const candidate_list_t *list, const eojeol_t *eojeol);
static void free_candidate_list(candidate_list_t *list);
static int str_vec_push_owned(str_vec_t *vec, char *item);
static void str_vec_free(str_vec_t *vec);

#endif
