#include "simti.h"

static simti_t * HANNANUM_UNUSED
simti_create(void)
{
  return (simti_t *)calloc(1, sizeof(simti_t));
}

static void
simti_node_free(simti_node_t *node)
{
  simti_node_t *child;
  if (node == NULL) {
    return;
  }
  child = node->child;
  while (child != NULL) {
    simti_node_t *next = child->sibling;
    simti_node_free(child);
    free(child);
    child = next;
  }
}

static void HANNANUM_UNUSED
simti_destroy(simti_t *simti)
{
  if (simti == NULL) {
    return;
  }
  simti_node_free(&simti->root);
  free(simti);
}

static void HANNANUM_UNUSED
simti_init(simti_t *simti)
{
  if (simti == NULL) {
    return;
  }
  simti_node_free(&simti->root);
  memset(simti, 0, sizeof(*simti));
}

static simti_node_t *
simti_look(simti_node_t *parent, unsigned int key)
{
  simti_node_t *node;
  if (parent == NULL) {
    return NULL;
  }
  for (node = parent->child; node != NULL; node = node->sibling) {
    if (node->key == key) {
      return node;
    }
    if (node->key > key) {
      break;
    }
  }
  return NULL;
}

static simti_node_t *
simti_insert_child(simti_node_t *parent, unsigned int key)
{
  simti_node_t *node;
  simti_node_t *current;
  simti_node_t *previous = NULL;
  if (parent == NULL) {
    return NULL;
  }
  current = parent->child;
  while (current != NULL && current->key < key) {
    previous = current;
    current = current->sibling;
  }
  if (current != NULL && current->key == key) {
    return current;
  }
  node = (simti_node_t *)calloc(1, sizeof(simti_node_t));
  if (node == NULL) {
    return NULL;
  }
  node->key = key;
  node->sibling = current;
  if (previous == NULL) {
    parent->child = node;
  } else {
    previous->sibling = node;
  }
  return node;
}

static size_t HANNANUM_UNUSED
simti_search(simti_t *simti, const unsigned int *word, size_t count)
{
  simti_node_t *node;
  size_t i = 0;
  if (simti == NULL || word == NULL) {
    return 0;
  }
  node = &simti->root;
  simti->search_end = 0;
  while (i < count) {
    node = simti_look(node, word[i]);
    if (node == NULL) {
      break;
    }
    if (simti->search_end < 1024) {
      simti->search_word[simti->search_end] = word[i];
      simti->search_nodes[simti->search_end] = node;
      simti->search_end++;
    }
    i++;
  }
  return simti->search_end;
}

static int HANNANUM_UNUSED
simti_fetch(simti_t *simti, const unsigned int *word, size_t count)
{
  if (simti == NULL || word == NULL || count == 0) {
    return 0;
  }
  if (simti_search(simti, word, count) != count) {
    return 0;
  }
  return simti->search_nodes[simti->search_end - 1]->info;
}

static int HANNANUM_UNUSED
simti_insert(simti_t *simti, const unsigned int *word, size_t count, int info)
{
  simti_node_t *node;
  size_t i;
  if (simti == NULL || word == NULL || count == 0) {
    return -1;
  }
  node = &simti->root;
  simti->search_end = 0;
  for (i = 0; i < count; i++) {
    node = simti_insert_child(node, word[i]);
    if (node == NULL) {
      return -1;
    }
    if (simti->search_end < 1024) {
      simti->search_word[simti->search_end] = word[i];
      simti->search_nodes[simti->search_end] = node;
      simti->search_end++;
    }
  }
  if (node->info == 0) {
    node->info = info;
    return 1;
  }
  return 0;
}
