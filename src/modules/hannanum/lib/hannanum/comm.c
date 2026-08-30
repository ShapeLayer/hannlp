static void
free_eojeol(eojeol_t * e)
{
  size_t          i;
  if (e == NULL) {
    return;
  }
  for (i = 0; i < e->length; i++) {
    free(e->morphemes[i]);
    free(e->tags[i]);
  }
  free(e->morphemes);
  free(e->tags);
  e->morphemes = NULL;
  e->tags = NULL;
  e->length = 0;
}
static eojeol_t clone_eojeol(const eojeol_t * src){
  eojeol_t        dst;
  size_t          i;
  dst.morphemes = NULL;
  dst.tags = NULL;
  dst.length = 0;
  if (src == NULL || src->length == 0) {
    return dst;
  }
  dst.morphemes = (char **)calloc(src->length, sizeof(char *));
  dst.tags = (char **)calloc(src->length, sizeof(char *));
  if (dst.morphemes == NULL || dst.tags == NULL) {
    free(dst.morphemes);
    free(dst.tags);
    dst.morphemes = NULL;
    dst.tags = NULL;
    return dst;
  }
  dst.length = src->length;
  for (i = 0; i < src->length; i++) {
    dst.morphemes[i] = hn_strdup(src->morphemes[i]);
    dst.tags[i] = hn_strdup(src->tags[i]);
    if (dst.morphemes[i] == NULL || dst.tags[i] == NULL) {
      free_eojeol(&dst);
      break;
    }
  }
  return dst;
}

static int
candidate_list_add(candidate_list_t * list, eojeol_t eojeol)
{
  eojeol_t       *next;
  if (!postprocess_eojeol(&eojeol)) {
    free_eojeol(&eojeol);
    return 0;
  }
  if (list->count == list->capacity) {
    size_t          new_capacity = list->capacity == 0 ? 2 : list->capacity * 2;
    next = (eojeol_t *) realloc(list->items, new_capacity * sizeof(eojeol_t));
    if (next == NULL) {
      return 0;
    }
    list->items = next;
    list->capacity = new_capacity;
  }
  list->items[list->count++] = eojeol;
  return 1;
}

static int
candidate_list_has_signature(const candidate_list_t * list, const eojeol_t * eojeol)
{
  size_t          i;
  if (list == NULL || eojeol == NULL) {
    return 0;
  }
  for (i = 0; i < list->count; i++) {
    size_t          j;
    if (list->items[i].length != eojeol->length) {
      continue;
    }
    for (j = 0; j < eojeol->length; j++) {
      if (strcmp(list->items[i].morphemes[j], eojeol->morphemes[j]) != 0 || strcmp(list->items[i].tags[j], eojeol->tags[j]) != 0) {
        break;
      }
    }
    if (j == eojeol->length) {
      return 1;
    }
  }
  return 0;
}

static void
free_candidate_list(candidate_list_t * list)
{
  size_t          i;
  if (list == NULL) {
    return;
  }
  for (i = 0; i < list->count; i++) {
    free_eojeol(&list->items[i]);
  }
  free(list->items);
  list->items = NULL;
  list->count = 0;
  list->capacity = 0;
}

static int
str_vec_push_owned(str_vec_t * vec, char *item)
{
  char          **next;
  if (vec->count == vec->capacity) {
    size_t          new_capacity = vec->capacity == 0 ? 8 : vec->capacity * 2;
    next = (char **)realloc(vec->items, new_capacity * sizeof(char *));
    if (next == NULL) {
      return 0;
    }
    vec->items = next;
    vec->capacity = new_capacity;
  }
  vec->items[vec->count++] = item;
  return 1;
}

static void
str_vec_free(str_vec_t * vec)
{
  size_t          i;
  if (vec == NULL) {
    return;
  }
  for (i = 0; i < vec->count; i++) {
    free(vec->items[i]);
  }
  free(vec->items);
  vec->items = NULL;
  vec->count = 0;
  vec->capacity = 0;
}
