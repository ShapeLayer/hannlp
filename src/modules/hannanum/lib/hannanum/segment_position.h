// derived from ChartMorphAnalyzer/SegmentPosition

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


#ifndef HANNANUM_SEGMENT_POSITION_H
#define HANNANUM_SEGMENT_POSITION_H

#define HANNANUM_MAX_SEGMENT 1024
#define HANNANUM_MAX_MORPHEME_COUNT 512
#define HANNANUM_SP_STATE_N 0
#define HANNANUM_SP_STATE_D 1
#define HANNANUM_SP_STATE_R 2
#define HANNANUM_SP_STATE_M 3
#define HANNANUM_SP_STATE_F 4
#define HANNANUM_POSITION_START_KEY 0u

typedef struct segment_position_node {
  unsigned int key;
  int state;
  int next_position;
  int s_index;
  int u_index;
  int n_index;
  int morph_count;
  int morpheme[HANNANUM_MAX_MORPHEME_COUNT];
} segment_position_node_t;

typedef struct segment_position {
  segment_position_node_t positions[HANNANUM_MAX_SEGMENT];
  int position_end;
} segment_position_t;

static void segment_position_clear(segment_position_t *sp);
static int segment_position_add(segment_position_t *sp, unsigned int key);
static segment_position_node_t *segment_position_get(segment_position_t *sp, int index);
static int segment_position_next(const segment_position_t *sp, int index);
static int segment_position_set_link(segment_position_t *sp, int previous, int next);
static int segment_position_init_from_triple(segment_position_t *sp, const codepoint_vec_t *triple);
static int segment_position_init_from_triple_with_simti(segment_position_t *sp, const codepoint_vec_t *triple, simti_t *simti);
static int segment_position_init_text(segment_position_t *sp, const char *text);

#endif
