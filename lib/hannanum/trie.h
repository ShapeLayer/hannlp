// derived from ChartMorphAnalyzer/Trie

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


#ifndef HANNANUM_TRIE_H
#define HANNANUM_TRIE_H

typedef struct trie_info {
  int tag;
  int phoneme;
  struct trie_info *next;
} trie_info_t;

typedef struct trie_node {
  unsigned int key;
  struct trie_node *child;
  struct trie_node *sibling;
  trie_info_t *info;
} trie_node_t;

typedef struct hannanum_trie {
  trie_node_t root;
  trie_node_t *search_nodes[256];
  unsigned int search_keys[256];
  size_t search_end;
} hannanum_trie_t;

static hannanum_trie_t *trie_create(void);
static void trie_destroy(hannanum_trie_t *trie);
static trie_node_t *trie_search_codepoints(hannanum_trie_t *trie, const unsigned int *word, size_t count);
static trie_node_t *trie_node_look_node(trie_node_t *parent, unsigned int key);
static int trie_store_codepoints(hannanum_trie_t *trie, const unsigned int *word, size_t count, int tag, int phoneme);
static int trie_store_text(hannanum_trie_t *trie, const char *word, int tag, int phoneme);
static trie_node_t *trie_fetch_text(hannanum_trie_t *trie, const char *word);

#endif
