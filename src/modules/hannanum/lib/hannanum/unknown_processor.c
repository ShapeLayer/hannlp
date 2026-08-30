static int
unknown_processor_expand_list(candidate_list_t *list)
{
  size_t original_count;
  size_t i;
  if (list == NULL) {
    return 0;
  }
  original_count = list->count;
  for (i = 0; i < original_count; i++) {
    size_t j;
    for (j = 0; j < list->items[i].length; j++) {
      if (strcmp(list->items[i].tags[j], "unk") == 0) {
        eojeol_t clone = clone_eojeol(&list->items[i]);
        if (clone.length > 0) {
          free(clone.tags[j]);
          clone.tags[j] = hn_strdup("nqq");
          if (clone.tags[j] == NULL || !candidate_list_add(list, clone)) {
            free_eojeol(&clone);
          }
        }
        free(list->items[i].tags[j]);
        list->items[i].tags[j] = hn_strdup("ncn");
      }
    }
  }
  return 1;
}

static candidate_list_t candidates_for(hannanum_t * h, const char *plain){
  candidate_list_t list;
  dict_entry_t   *entry;
  size_t          i;
  memset(&list, 0, sizeof(list));
  entry = dict_find(h->dict, plain);
  if (entry != NULL && entry->analyzed_count > 0) {
    for (i = 0; i < entry->analyzed_count; i++) {
      eojeol_t        e = clone_eojeol(&entry->candidates.items[i]);
      if (e.length > 0 && !candidate_list_add(&list, e)) {
        free_eojeol(&e);
      }
    }
    return list;
  }
  list = chart_candidates_for(h, plain);
  unknown_processor_expand_list(&list);
  if (list.count > 0) {
    return list;
  }
  if (list.count == 0) {
    eojeol_t        e;
    e = make_single(plain, "ncn");
    if (e.length > 0) {
      candidate_list_add(&list, e);
    }
    e = make_single(plain, "nqq");
    if (e.length > 0) {
      candidate_list_add(&list, e);
    }
  }
  return list;
}
