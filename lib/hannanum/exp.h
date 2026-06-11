// derived from ChartMorphAnalyzer/Exp

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


#ifndef HANNANUM_EXP_H
#define HANNANUM_EXP_H

#define EXP_TAG_TYPE_YONGS 7
#define EXP_TAG_TYPE_EOMIES 8
#define EXP_TAG_TYPE_JP 9
#define EXP_TAG_TYPE_NBNP 5
#define EXP_TAG_TYPE_JOSA 6

typedef struct exp_change {
  codepoint_vec_t front;
  codepoint_vec_t back;
  int front_tag_type;
  int back_tag_type;
  int phoneme;
} exp_change_t;

typedef struct exp_irregular_ids {
  int type_b;
  int type_s;
  int type_d;
  int type_h;
  int type_reu;
  int type_reo;
} exp_irregular_ids_t;

static void exp_change_free(exp_change_t *change);
static size_t exp_prule_generate(const codepoint_vec_t *prev, const codepoint_vec_t *str, const exp_irregular_ids_t *ids, exp_change_t *changes, size_t max_count);

#ifdef HANNANUM_WITH_CHART_BRIDGE
static int exp_apply_changes_to_chart(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int from, const exp_change_t *changes, size_t count);
#endif

#endif
