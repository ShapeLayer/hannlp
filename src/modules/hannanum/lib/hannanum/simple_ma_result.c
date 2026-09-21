static int
candidate_signature(const eojeol_t *eojeol, char **out)
{
  size_t i;
  struct strbuffer signature;
  strbuffer_init(&signature, 64);
  for (i = 0; i < eojeol->length; i++) {
    if (i != 0) {
      strbuffer_add_byte(&signature, '+');
    }
    strbuffer_add_str(&signature, eojeol->morphemes[i]);
    strbuffer_add_byte(&signature, '/');
    strbuffer_add_str(&signature, eojeol->tags[i]);
  }
  *out = (char *)strbuffer_steal(&signature);
  return *out != NULL;
}

/* Return -1 on allocation failure, 0 if absent, and 1 if present. */
static int
candidate_list_contains_signature(candidate_list_t *list, const eojeol_t *candidate)
{
  char *candidate_sig;
  size_t i;
  if (!candidate_signature(candidate, &candidate_sig)) {
    return -1;
  }
  for (i = 0; i < list->count; i++) {
    char *sig;
    int equal;
    if (!candidate_signature(&list->items[i], &sig)) {
      free(candidate_sig);
      return -1;
    }
    equal = strcmp(candidate_sig, sig) == 0;
    free(sig);
    if (equal) {
      free(candidate_sig);
      return 1;
    }
  }
  free(candidate_sig);
  return 0;
}

static int
simple_ma_process_list(candidate_list_t *list, int level)
{
  candidate_list_t result;
  size_t i;
  memset(&result, 0, sizeof(result));
  for (i = 0; i < list->count; i++) {
    int contains;
    eojeol_t candidate = clone_eojeol(&list->items[i]);
    if (candidate.length == 0) {
      continue;
    }
    if (!simple_pos_process_eojeol(&candidate, level)) {
      free_candidate_list(&result);
      free_eojeol(&candidate);
      return 0;
    }
    contains = candidate_list_contains_signature(&result, &candidate);
    if (contains < 0) {
      free_candidate_list(&result);
      free_eojeol(&candidate);
      return 0;
    }
    if (!contains) {
      if (!candidate_list_add(&result, candidate)) {
        free_candidate_list(&result);
        free_eojeol(&candidate);
        return 0;
      }
    } else {
      free_eojeol(&candidate);
    }
  }
  free_candidate_list(list);
  *list = result;
  return 1;
}
