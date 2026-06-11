// derived from ChartMorphAnalyzer/MorphemeChart

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


#ifndef HANNANUM_MORPHEME_CHART_H
#define HANNANUM_MORPHEME_CHART_H

#define HANNANUM_MAX_MORPHEME_CONNECTION 30
#define HANNANUM_MAX_MORPHEME_CHART 2046
#define HANNANUM_MORPHEME_STATE_FAIL 0
#define HANNANUM_MORPHEME_STATE_SUCCESS 1
#define HANNANUM_MORPHEME_STATE_INCOMPLETE 2

#include "number_dic.h"

typedef struct morpheme_chart_node {
  int tag;
  int phoneme;
  int next_position;
  int next_tag_type;
  int state;
  int connection_count;
  int connection[HANNANUM_MAX_MORPHEME_CONNECTION];
  char *str;
} morpheme_chart_node_t;

typedef struct morpheme_chart {
  morpheme_chart_node_t nodes[HANNANUM_MAX_MORPHEME_CHART];
  int chart_end;
  int eng_replacement_count;
  int chi_replacement_count;
  char *eng_replacements[128];
  char *chi_replacements[128];
} morpheme_chart_t;

static void morpheme_chart_clear(morpheme_chart_t *chart);
static int morpheme_chart_init(morpheme_chart_t *chart);
static char *morpheme_chart_pre_replace(morpheme_chart_t *chart, const char *input);
static int morpheme_chart_init_word(morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, const char *word, int iwg_tag);
static int morpheme_chart_add(morpheme_chart_t *chart, int tag, int phoneme, int next_position, int next_tag_type, const char *str);
static int morpheme_chart_add_connection(morpheme_chart_t *chart, int from, int to);
static int morpheme_chart_check(const morpheme_chart_t *chart, const int *morphemes, int morpheme_len, int tag, int phoneme, int next_position, int next_tag_type, const char *str);
static int morpheme_chart_node_to_eojeol(const morpheme_chart_t *chart, const int *path, size_t path_len, eojeol_t *out, char **tag_names, size_t tag_count);
static int morpheme_chart_alt_segment(segment_position_t *sp, simti_t *simti, const codepoint_vec_t *segment);
static int morpheme_chart_phoneme_change(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int from, const codepoint_vec_t *front, const codepoint_vec_t *back, int front_tag_type, int back_tag_type, int phoneme);
static int morpheme_chart_scan_dictionaries(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int chart_index);
typedef int (*morpheme_chart_expander_t)(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int from, const char *morph_text, void *userdata);
static int morpheme_chart_expand_d_state(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int chart_index, morpheme_chart_expander_t expander, void *userdata);
typedef int (*morpheme_chart_connector_t)(hannanum_t *h, const morpheme_chart_node_t *left, const morpheme_chart_node_t *right, void *userdata);
static int morpheme_chart_connect_m_state(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int chart_index, morpheme_chart_connector_t connector, void *userdata);
typedef int (*morpheme_chart_tag_type_t)(hannanum_t *h, int tag_type, int tag, void *userdata);
typedef int (*morpheme_chart_recursor_t)(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int chart_index, int next_tag_type, void *userdata);
static int morpheme_chart_process_r_state(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int chart_index, int tag_type, morpheme_chart_tag_type_t tag_type_check, morpheme_chart_recursor_t recursor, void *userdata);
static int morpheme_chart_analyze_with_callbacks(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int chart_index, int tag_type, morpheme_chart_expander_t expander, morpheme_chart_tag_type_t tag_type_check, morpheme_chart_connector_t connector, void *userdata);
static int morpheme_chart_collect_results(const morpheme_chart_t *chart, int chart_index, char **tag_names, size_t tag_count, candidate_list_t *out);
static int morpheme_chart_analyze_unknown(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int unk_tag);

#endif
