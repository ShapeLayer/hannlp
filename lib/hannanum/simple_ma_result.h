// derived from SimpleMAResult09/22 morpheme processors.

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


#ifndef HANNANUM_SIMPLE_MA_RESULT_H
#define HANNANUM_SIMPLE_MA_RESULT_H

static int candidate_signature(const eojeol_t *eojeol, char **out);
static int candidate_list_contains_signature(candidate_list_t *list, const eojeol_t *candidate);
static int simple_ma_process_list(candidate_list_t *list, int level);

#endif
