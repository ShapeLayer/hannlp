#ifndef HANNANUM_SIMTI_H
#define HANNANUM_SIMTI_H

/* Port of ChartMorphAnalyzer/Simti.java reverse substring index. */

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
