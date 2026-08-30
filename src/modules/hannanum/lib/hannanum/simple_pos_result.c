static char *
tag_on_level(const char *tag, int level)
{
  size_t len;
  size_t out_len;
  size_t i;
  struct strbuffer mapped;
  if (tag == NULL || level < 1 || level > 4) {
    return NULL;
  }
  len = strlen(tag);
  out_len = len > (size_t)level ? (size_t)level : len;
  strbuffer_init(&mapped, (size_t)out_len);
  for (i = 0; i < out_len; i++) {
    strbuffer_add_byte(&mapped, (unsigned char)toupper((unsigned char)tag[i]));
  }
  return (char *)strbuffer_steal(&mapped);
}

static int
simple_pos_process_eojeol(eojeol_t *eojeol, int level)
{
  char **new_morphemes;
  char **new_tags;
  size_t new_count = 0;
  size_t i;
  if (eojeol == NULL || eojeol->length == 0) {
    return 1;
  }
  new_morphemes = (char **)calloc(eojeol->length, sizeof(char *));
  new_tags = (char **)calloc(eojeol->length, sizeof(char *));
  if (new_morphemes == NULL || new_tags == NULL) {
    free(new_morphemes);
    free(new_tags);
    return 0;
  }
  for (i = 0; i < eojeol->length; i++) {
    char *mapped = tag_on_level(eojeol->tags[i], level);
    if (mapped == NULL) {
      goto fail;
    }
    if (new_count > 0 && strcmp(new_tags[new_count - 1], mapped) == 0) {
      if (!hn_str_append_str(&new_morphemes[new_count - 1], eojeol->morphemes[i])) {
        free(mapped);
        goto fail;
      }
      free(mapped);
    } else {
      new_morphemes[new_count] = hn_strdup(eojeol->morphemes[i]);
      new_tags[new_count] = mapped;
      if (new_morphemes[new_count] == NULL) {
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
simple_pos_process_result(hannanum_result_t *result, int level)
{
  size_t i;
  if (result == NULL) {
    return 0;
  }
  for (i = 0; i < result->count; i++) {
    if (!simple_pos_process_eojeol(&result->eojeols[i], level)) {
      return 0;
    }
  }
  return 1;
}
