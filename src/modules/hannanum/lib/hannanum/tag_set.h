// derived from ChartMorphAnalyzer/TagSet

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


#ifndef HANNANUM_TAGSET_H
#define HANNANUM_TAGSET_H

static int tag_id(hannanum_t *h, const char *tag);
static int irregular_id(hannanum_t *h, const char *name);
static int load_tag_set(hannanum_t *h);
static void free_tag_set(hannanum_t *h);
static int check_tag_type(hannanum_t *h, int type, int tag);
static int check_phoneme_type(int phoneme_type, int phoneme);

#endif
