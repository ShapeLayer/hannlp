#include "trie.h"

static hannanum_trie_t * HANNANUM_UNUSED
trie_create(void)
{
  hannanum_trie_t *trie = (hannanum_trie_t *)calloc(1, sizeof(hannanum_trie_t));
  return trie;
}

static void
trie_info_free(trie_info_t *info)
{
  while (info != NULL) {
    trie_info_t *next = info->next;
    free(info);
    info = next;
  }
}

static void
trie_node_free_children(trie_node_t *node)
{
  trie_node_t *child;
  if (node == NULL) {
    return;
  }
  child = node->child;
  while (child != NULL) {
    trie_node_t *next = child->sibling;
    trie_node_free_children(child);
    trie_info_free(child->info);
    free(child);
    child = next;
  }
}

static void HANNANUM_UNUSED
trie_destroy(hannanum_trie_t *trie)
{
  if (trie == NULL) {
    return;
  }
  trie_node_free_children(&trie->root);
  trie_info_free(trie->root.info);
  free(trie);
}

static trie_node_t *
trie_node_look_node(trie_node_t *parent, unsigned int key)
{
  trie_node_t *node;
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

static trie_node_t *
trie_node_insert_child(trie_node_t *parent, unsigned int key)
{
  trie_node_t *node;
  trie_node_t *prev = NULL;
  trie_node_t *current;
  if (parent == NULL) {
    return NULL;
  }
  current = parent->child;
  while (current != NULL && current->key < key) {
    prev = current;
    current = current->sibling;
  }
  if (current != NULL && current->key == key) {
    return current;
  }
  node = (trie_node_t *)calloc(1, sizeof(trie_node_t));
  if (node == NULL) {
    return NULL;
  }
  node->key = key;
  node->sibling = current;
  if (prev == NULL) {
    parent->child = node;
  } else {
    prev->sibling = node;
  }
  return node;
}

static trie_node_t *
trie_search_codepoints(hannanum_trie_t *trie, const unsigned int *word, size_t count)
{
  trie_node_t *node;
  size_t i;
  if (trie == NULL || word == NULL || count == 0) {
    return NULL;
  }
  node = &trie->root;
  trie->search_end = 0;
  for (i = 0; i < count; i++) {
    node = trie_node_look_node(node, word[i]);
    if (node == NULL) {
      return NULL;
    }
    if (trie->search_end < 256) {
      trie->search_nodes[trie->search_end] = node;
      trie->search_keys[trie->search_end] = word[i];
      trie->search_end++;
    }
  }
  return node->info != NULL ? node : NULL;
}

static int
trie_store_codepoints(hannanum_trie_t *trie, const unsigned int *word, size_t count, int tag, int phoneme)
{
  trie_node_t *node;
  trie_info_t *info;
  size_t i;
  if (trie == NULL || word == NULL || count == 0) {
    return 0;
  }
  node = &trie->root;
  trie->search_end = 0;
  for (i = 0; i < count; i++) {
    node = trie_node_insert_child(node, word[i]);
    if (node == NULL) {
      return 0;
    }
    if (trie->search_end < 256) {
      trie->search_nodes[trie->search_end] = node;
      trie->search_keys[trie->search_end] = word[i];
      trie->search_end++;
    }
  }
  info = (trie_info_t *)calloc(1, sizeof(trie_info_t));
  if (info == NULL) {
    return 0;
  }
  info->tag = tag;
  info->phoneme = phoneme;
  info->next = NULL;
  if (node->info == NULL) {
    node->info = info;
  } else {
    trie_info_t *tail = node->info;
    while (tail->next != NULL) {
      tail = tail->next;
    }
    tail->next = info;
  }
  return 1;
}

static int HANNANUM_UNUSED
trie_store_text(hannanum_trie_t *trie, const char *word, int tag, int phoneme)
{
  codepoint_vec_t triple;
  int ok;
  memset(&triple, 0, sizeof(triple));
  if (!hannanum_code_to_triple(word, &triple)) {
    codepoint_vec_free(&triple);
    return 0;
  }
  ok = trie_store_codepoints(trie, triple.items, triple.count, tag, phoneme);
  codepoint_vec_free(&triple);
  return ok;
}

static trie_node_t HANNANUM_UNUSED *
trie_fetch_text(hannanum_trie_t *trie, const char *word)
{
  codepoint_vec_t triple;
  trie_node_t *node;
  memset(&triple, 0, sizeof(triple));
  if (!hannanum_code_to_triple(word, &triple)) {
    codepoint_vec_free(&triple);
    return NULL;
  }
  node = trie_search_codepoints(trie, triple.items, triple.count);
  codepoint_vec_free(&triple);
  return node;
}
