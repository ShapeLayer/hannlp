// Derived from ChartMorphAnalyzer/Connection

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


#ifndef HANNANUM_CONNECTION_H
#define HANNANUM_CONNECTION_H

static int load_connections(hannanum_t *h);
static int connection_allows(hannanum_t *h, const char *left_tag, const char *right_tag);
static void free_connections(hannanum_t *h);

#endif
