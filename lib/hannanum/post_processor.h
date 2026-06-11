// derived from ChartMorphAnalyzer/PostProcessor

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


#ifndef HANNANUM_POST_PROCESSOR_H
#define HANNANUM_POST_PROCESSOR_H

static int replace_morpheme(eojeol_t *e, size_t index, const char *morpheme);
static char *drop_initial_eu_from_morpheme(const char *morpheme);
static int can_drop_initial_eu(const char *morpheme);
static int postprocess_eojeol(eojeol_t *e);

#endif
