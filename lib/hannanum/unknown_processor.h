// derived from SupplementPlugin/MorphemeProcessor/UnknownMorphProcessor/UnknownProcessor

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


#ifndef HANNANUM_UNKNOWN_PROCESSOR_H
#define HANNANUM_UNKNOWN_PROCESSOR_H

static candidate_list_t candidates_for(hannanum_t *h, const char *plain);
static int unknown_processor_expand_list(candidate_list_t *list);

#endif
