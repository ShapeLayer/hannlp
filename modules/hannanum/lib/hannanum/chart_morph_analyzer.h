// derived from MajorPlugin/MorphAnalyzer/ChartMorphAnalyzer 

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


#ifndef HANNANUM_CHART_MORPH_ANALYZER_H
#define HANNANUM_CHART_MORPH_ANALYZER_H

static dict_entry_t *dict_find(dict_entry_t **dict, const char *key);
static dict_entry_t *dict_get_or_create(hannanum_t *h, const char *key);
static void prob_put(prob_entry_t **table, const char *key, double value);
static int prob_get(prob_entry_t **table, const char *key, double *value);
static int parse_analysis(const char *analysis, eojeol_t *out);
static eojeol_t make_single(const char *surface, const char *tag);
static eojeol_t make_pair(const char *first_surface, const char *first_tag, const char *second_surface, const char *second_tag);
static eojeol_t make_parts(size_t count, const char **surfaces, const char **tags);
static int is_utf8_continuation(unsigned char c);
static int segment_stack_push(segment_stack_t *stack, const char *morpheme, const char *tag);
static void segment_stack_pop(segment_stack_t *stack);
static void segment_stack_free(segment_stack_t *stack);
static eojeol_t eojeol_from_stack(const segment_stack_t *stack);
static int segment_tag_allowed(const char *tag, int at_start, int at_end);
static int segment_connection_ok(hannanum_t *h, const char *prev, const char *next);
static void add_segmented_candidate(candidate_list_t *out, const segment_stack_t *stack);
static void segment_dfs(hannanum_t *h, const char *plain, size_t offset, segment_stack_t *stack, candidate_list_t *out);
static void load_analyzed_dic(hannanum_t *h);
static void load_surface_dic(hannanum_t *h, const char *relative, hannanum_trie_t *trie);
static void load_probability(hannanum_t *h, prob_entry_t **table, const char *relative);
static int is_repeated_sentence_punctuation(const char *s);
static int is_ascii_digits(const char *s, size_t len);
static int is_numeric_expression(const char *s);
static int is_simple_signed_digits(const char *s);
static int is_ascii_abbreviation(const char *s);
static int is_ascii_alpha_token(const char *s);
static int is_number_unit(const char *s);
static int korean_number_syllable_tag(const char *syllable, int has_previous, const char **tag);
static int entry_has_noun_tag(const dict_entry_t *entry);
static const char *entry_preferred_noun_tag(const dict_entry_t *entry);
static int append_parenthesized_prefix_candidate(hannanum_t *h, const char *plain, candidate_list_t *list);
static int entry_has_yong_tag(const dict_entry_t *entry);
static const char *entry_preferred_yong_tag(const dict_entry_t *entry);
static int append_stem_ending_candidate(hannanum_t *h, candidate_list_t *list, const char *stem, const char *ending, const char *ending_tag);
static char *stem_without_final_jong(const char *plain, size_t syllable_offset, size_t syllable_width);
static int chart_tag_type_check(hannanum_t *h, int tag_type, int tag, void *userdata);
static int chart_connection_check(hannanum_t *h, const morpheme_chart_node_t *left, const morpheme_chart_node_t *right, void *userdata);
static int chart_remaining_from_position(segment_position_t *sp, int from, codepoint_vec_t *remaining);
static int chart_expander(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int from, const char *morph_text, void *userdata);
static candidate_list_t chart_candidates_for(hannanum_t *h, const char *plain) HANNANUM_UNUSED;

#endif
