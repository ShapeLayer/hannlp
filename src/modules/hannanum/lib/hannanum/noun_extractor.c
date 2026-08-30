static int
noun_extractor_process_eojeol(eojeol_t *eojeol)
{
  char **new_morphemes;
  char **new_tags;
  size_t new_count = 0;
  size_t i;
  if (eojeol == NULL) {
    return 0;
  }
  new_morphemes = (char **)calloc(eojeol->length == 0 ? 1 : eojeol->length, sizeof(char *));
  new_tags = (char **)calloc(eojeol->length == 0 ? 1 : eojeol->length, sizeof(char *));
  if (new_morphemes == NULL || new_tags == NULL) {
    free(new_morphemes);
    free(new_tags);
    return 0;
  }
  for (i = 0; i < eojeol->length; i++) {
    const char *tag = eojeol->tags[i];
    if (tag[0] == 'n' || tag[0] == 'f') {
      new_morphemes[new_count] = hn_strdup(eojeol->morphemes[i]);
      new_tags[new_count] = hn_strdup(tag[0] == 'f' ? "ncn" : tag);
      if (new_morphemes[new_count] == NULL || new_tags[new_count] == NULL) {
        goto fail;
      }
      new_count++;
    }
  }
  free_eojeol(eojeol);
  eojeol->morphemes = new_morphemes;
  eojeol->tags = new_tags;
  eojeol->length = new_count;
  return 1;

fail:
  for (i = 0; i < eojeol->length; i++) {
    free(new_morphemes[i]);
    free(new_tags[i]);
  }
  free(new_morphemes);
  free(new_tags);
  return 0;
}

static int
noun_extractor_process_result(hannanum_result_t *result)
{
  size_t i;
  if (result == NULL) {
    return 0;
  }
  for (i = 0; i < result->count; i++) {
    if (!noun_extractor_process_eojeol(&result->eojeols[i])) {
      return 0;
    }
  }
  return 1;
}
