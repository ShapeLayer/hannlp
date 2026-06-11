static int
replace_morpheme(eojeol_t * e, size_t index, const char *morpheme)
{
  char           *replacement = strbuf_strdup(morpheme);
  if (replacement == NULL) {
    return 0;
  }
  free(e->morphemes[index]);
  e->morphemes[index] = replacement;
  return 1;
}

static char *
drop_initial_eu_from_morpheme(const char *morpheme)
{
  unsigned int cp;
  size_t width;
  unsigned int index;
  unsigned int cho;
  unsigned int jung;
  unsigned int jong;
  char *out = NULL;
  size_t used = 0;
  size_t capacity = 0;
  if (!utf8_decode_one((const unsigned char *)morpheme, &cp, &width) || cp < 0xac00 || cp > 0xd7a3) {
    return NULL;
  }
  index = cp - 0xac00;
  cho = index / (21 * 28);
  jung = (index / 28) % 21;
  jong = index % 28;
  if (jung != 18) {
    return NULL;
  }
  if (jong != 0) {
    if (!strbuf_append_utf8_to_cstr(&out, &used, &capacity, jongseong_compat[jong])) {
      free(out);
      return NULL;
    }
  } else if (cho < sizeof(choseong_compat) / sizeof(choseong_compat[0])) {
    if (!strbuf_append_utf8_to_cstr(&out, &used, &capacity, choseong_compat[cho])) {
      free(out);
      return NULL;
    }
  }
  if (!strbuf_append_to_cstr(&out, &used, &capacity, morpheme + width, strlen(morpheme + width))) {
    free(out);
    return NULL;
  }
  return out;
}

static int
can_drop_initial_eu(const char *morpheme)
{
  char *contracted = drop_initial_eu_from_morpheme(morpheme);
  if (contracted == NULL) {
    return 0;
  }
  free(contracted);
  return 1;
}

static int
postprocess_eojeol(eojeol_t * e)
{
  size_t          i;
  const char     *prev = "";
  if (e == NULL) {
    return 0;
  }
  for (i = 0; i < e->length; i++) {
    if (starts_with(e->tags[i], "e")) {
      if ((starts_with(e->morphemes[i], "어") || starts_with(e->morphemes[i], "었")) && hangul_has_positive_vowel(prev) && strcmp(prev, "하") != 0) {
        if (starts_with(e->morphemes[i], "었")) {
          if (!replace_morpheme(e, i, "아")) {
            return 0;
          }
        } else if (starts_with(e->morphemes[i], "어")) {
          if (!replace_morpheme(e, i, "아")) {
            return 0;
          }
        }
      } else if ((can_drop_initial_eu(e->morphemes[i]) || starts_with(e->morphemes[i], "으") || starts_with(e->morphemes[i], "스") || starts_with(e->morphemes[i], "느")) && hangul_final_is_vowel_or_l(prev)) {
        char *contracted = drop_initial_eu_from_morpheme(e->morphemes[i]);
        if (contracted != NULL) {
          free(e->morphemes[i]);
          e->morphemes[i] = contracted;
        }
      }
    }
    prev = e->morphemes[i];
  }
  return 1;
}
