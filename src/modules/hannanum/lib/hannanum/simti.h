// derived from ChartMorphAnalyzer/Simti

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


#ifndef HANNANUM_SIMTI_H
#define HANNANUM_SIMTI_H

typedef struct simti_node {
  unsigned int key;
  int info;
  struct simti_node *child;
  struct simti_node *sibling;
} simti_node_t;

typedef struct simti {
  simti_node_t root;
  simti_node_t *search_nodes[1024];
  unsigned int search_word[1024];
  size_t search_end;
} simti_t;

static simti_t *simti_create(void);
static void simti_destroy(simti_t *simti);
static void simti_init(simti_t *simti);
static size_t simti_search(simti_t *simti, const unsigned int *word, size_t count);
static int simti_fetch(simti_t *simti, const unsigned int *word, size_t count);
static int simti_insert(simti_t *simti, const unsigned int *word, size_t count, int info);

#endif
