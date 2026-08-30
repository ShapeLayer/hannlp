hannanum_t     *
hannanum_create(const hannanum_options_t * options)
{
  hannanum_t     *h = (hannanum_t *) calloc(1, sizeof(hannanum_t));
  if (h == NULL) {
    return NULL;
  }
  h->data_dir = hn_strdup(options != NULL && options->data_dir != NULL ? options->data_dir : "../JHanNanum-0.8.4-en/JHanNanum/data");
  h->output_mode = options != NULL ? options->output_mode : HANNANUM_OUTPUT_HMM_POS;
  h->dict = (dict_entry_t * *) calloc(HANNANUM_HASH_SIZE, sizeof(dict_entry_t *));
  h->pwt = (prob_entry_t * *) calloc(HANNANUM_HASH_SIZE, sizeof(prob_entry_t *));
  h->ptt_pos = (prob_entry_t * *) calloc(HANNANUM_HASH_SIZE, sizeof(prob_entry_t *));
  h->ptt_wp = (prob_entry_t * *) calloc(HANNANUM_HASH_SIZE, sizeof(prob_entry_t *));
  h->system_trie = trie_create();
  h->user_trie = trie_create();
  if (h->data_dir == NULL || h->dict == NULL || h->pwt == NULL || h->ptt_pos == NULL || h->ptt_wp == NULL || h->system_trie == NULL || h->user_trie == NULL) {
    hannanum_destroy(h);
    return NULL;
  }
  if (load_tag_set(h)) {
    load_connections(h);
  }
  load_analyzed_dic(h);
  load_surface_dic(h, "kE/dic_system.txt", h->system_trie);
  load_surface_dic(h, "kE/dic_user.txt", h->user_trie);
  load_probability(h, h->pwt, "stat/PWT.pos");
  load_probability(h, h->ptt_pos, "stat/PTT.pos");
  load_probability(h, h->ptt_wp, "stat/PTT.wp");
  return h;
}

void
hannanum_destroy(hannanum_t * h)
{
  size_t          i;
  if (h == NULL) {
    return;
  }
  if (h->dict != NULL) {
    for (i = 0; i < HANNANUM_HASH_SIZE; i++) {
      dict_entry_t   *entry = h->dict[i];
      while (entry != NULL) {
        dict_entry_t   *next = entry->next;
        free(entry->key);
        free_candidate_list(&entry->candidates);
        free(entry);
        entry = next;
      }
    }
    free(h->dict);
  }
  if (h->pwt != NULL || h->ptt_pos != NULL || h->ptt_wp != NULL) {
    prob_entry_t  **tables[3] = {h->pwt, h->ptt_pos, h->ptt_wp};
    size_t          t;
    for (t = 0; t < 3; t++) {
      if (tables[t] == NULL) {
        continue;
      }
      for (i = 0; i < HANNANUM_HASH_SIZE; i++) {
        prob_entry_t   *entry = tables[t][i];
        while (entry != NULL) {
          prob_entry_t   *next = entry->next;
          free(entry->key);
          free(entry);
          entry = next;
        }
      }
      free(tables[t]);
    }
  }
  free_connections(h);
  free_tag_set(h);
  trie_destroy(h->system_trie);
  trie_destroy(h->user_trie);
  free(h->data_dir);
  free(h);
}

const char     *
hannanum_error(const hannanum_t * hannanum)
{
  return hannanum != NULL ? hannanum->error : "hannanum context is null";
}

hannanum_result_t *
hannanum_analyze(hannanum_t * h, const char *input)
{
  str_vec_t       tokens;
  candidate_list_t *sets;
  size_t         *selected;
  hannanum_result_t *result;
  char           *filtered;
  size_t          i;
  if (h == NULL || input == NULL) {
    return NULL;
  }
  memset(&tokens, 0, sizeof(tokens));
  filtered = informal_sentence_filter(input);
  if (filtered == NULL) {
    set_error(h, "failed to filter informal input");
    return NULL;
  }
  if (!split_eojeols(filtered, &tokens)) {
    free(filtered);
    str_vec_free(&tokens);
    set_error(h, "failed to split input");
    return NULL;
  }
  free(filtered);
  result = (hannanum_result_t *) calloc(1, sizeof(hannanum_result_t));
  sets = (candidate_list_t *) calloc(tokens.count == 0 ? 1 : tokens.count, sizeof(candidate_list_t));
  selected = (size_t *) calloc(tokens.count == 0 ? 1 : tokens.count, sizeof(size_t));
  if (result == NULL || sets == NULL || selected == NULL) {
    free(result);
    free(sets);
    free(selected);
    str_vec_free(&tokens);
    set_error(h, "out of memory");
    return NULL;
  }
  for (i = 0; i < tokens.count; i++) {
    sets[i] = candidates_for(h, tokens.items[i]);
  }
  if (h->output_mode == HANNANUM_OUTPUT_MORPH || h->output_mode == HANNANUM_OUTPUT_MORPH_SIMPLE_09 || h->output_mode == HANNANUM_OUTPUT_MORPH_SIMPLE_22) {
    result = (hannanum_result_t *) calloc(1, sizeof(hannanum_result_t));
    if (result == NULL) {
      for (i = 0; i < tokens.count; i++) {
        free_candidate_list(&sets[i]);
      }
      free(sets);
      free(selected);
      str_vec_free(&tokens);
      set_error(h, "out of memory");
      return NULL;
    }
    result->count = tokens.count;
    result->plain = (char **)calloc(result->count == 0 ? 1 : result->count, sizeof(char *));
    result->candidate_sets = (candidate_list_t *)calloc(result->count == 0 ? 1 : result->count, sizeof(candidate_list_t));
    if (result->plain == NULL || result->candidate_sets == NULL) {
      hannanum_result_destroy(result);
      for (i = 0; i < tokens.count; i++) {
        free_candidate_list(&sets[i]);
      }
      free(sets);
      free(selected);
      str_vec_free(&tokens);
      set_error(h, "out of memory");
      return NULL;
    }
    for (i = 0; i < result->count; i++) {
      size_t j;
      result->plain[i] = hn_strdup(tokens.items[i]);
      for (j = 0; j < sets[i].count; j++) {
        eojeol_t candidate = clone_eojeol(&sets[i].items[j]);
        if (candidate.length > 0 && !candidate_list_add(&result->candidate_sets[i], candidate)) {
          free_eojeol(&candidate);
        }
      }
      if (h->output_mode == HANNANUM_OUTPUT_MORPH_SIMPLE_09 && !simple_ma_process_list(&result->candidate_sets[i], 1)) {
        hannanum_result_destroy(result);
        result = NULL;
        set_error(h, "failed to produce simple MA 09 result");
        break;
      }
      if (h->output_mode == HANNANUM_OUTPUT_MORPH_SIMPLE_22 && !simple_ma_process_list(&result->candidate_sets[i], 2)) {
        hannanum_result_destroy(result);
        result = NULL;
        set_error(h, "failed to produce simple MA 22 result");
        break;
      }
    }
    for (i = 0; i < tokens.count; i++) {
      free_candidate_list(&sets[i]);
    }
    free(sets);
    free(selected);
    str_vec_free(&tokens);
    return result;
  }
  if (!select_best_by_sentence(h, sets, tokens.count, selected)) {
    for (i = 0; i < tokens.count; i++) {
      free_candidate_list(&sets[i]);
    }
    free(sets);
    free(selected);
    str_vec_free(&tokens);
    free(result);
    set_error(h, "failed to select best candidate");
    return NULL;
  }
  result->count = tokens.count;
  result->plain = (char **)calloc(result->count == 0 ? 1 : result->count, sizeof(char *));
  result->eojeols = (eojeol_t *) calloc(result->count == 0 ? 1 : result->count, sizeof(eojeol_t));
  if (result->plain == NULL || result->eojeols == NULL) {
    hannanum_result_destroy(result);
    for (i = 0; i < tokens.count; i++) {
      free_candidate_list(&sets[i]);
    }
    free(sets);
    free(selected);
    str_vec_free(&tokens);
    set_error(h, "out of memory");
    return NULL;
  }
  for (i = 0; i < result->count; i++) {
    result->plain[i] = hn_strdup(tokens.items[i]);
    result->eojeols[i] = clone_eojeol(&sets[i].items[selected[i]]);
  }
  if (h->output_mode == HANNANUM_OUTPUT_SIMPLE_POS_09 && !simple_pos_process_result(result, 1)) {
    hannanum_result_destroy(result);
    result = NULL;
    set_error(h, "failed to produce simple POS 09 result");
  } else if (h->output_mode == HANNANUM_OUTPUT_SIMPLE_POS_22 && !simple_pos_process_result(result, 2)) {
    hannanum_result_destroy(result);
    result = NULL;
    set_error(h, "failed to produce simple POS 22 result");
  } else if (h->output_mode == HANNANUM_OUTPUT_NOUNS && !noun_extractor_process_result(result)) {
    hannanum_result_destroy(result);
    result = NULL;
    set_error(h, "failed to extract nouns");
  }
  for (i = 0; i < tokens.count; i++) {
    free_candidate_list(&sets[i]);
  }
  free(sets);
  free(selected);
  str_vec_free(&tokens);
  return result;
}

void
hannanum_result_destroy(hannanum_result_t * result)
{
  size_t          i;
  if (result == NULL) {
    return;
  }
  for (i = 0; i < result->count; i++) {
    free(result->plain[i]);
    free_eojeol(&result->eojeols[i]);
    if (result->candidate_sets != NULL) {
      free_candidate_list(&result->candidate_sets[i]);
    }
  }
  free(result->plain);
  free(result->eojeols);
  free(result->candidate_sets);
  free(result);
}

size_t
hannanum_result_eojeol_count(const hannanum_result_t * result)
{
  return result != NULL ? result->count : 0;
}

size_t
hannanum_result_morpheme_count(const hannanum_result_t * result, size_t eojeol_index)
{
  return result != NULL && eojeol_index < result->count ? result->eojeols[eojeol_index].length : 0;
}

const char     *
hannanum_result_plain(const hannanum_result_t * result, size_t eojeol_index)
{
  return result != NULL && eojeol_index < result->count ? result->plain[eojeol_index] : NULL;
}

const char     *
hannanum_result_morpheme(const hannanum_result_t * result, size_t eojeol_index, size_t morpheme_index)
{
  if (result == NULL || eojeol_index >= result->count || morpheme_index >= result->eojeols[eojeol_index].length) {
    return NULL;
  }
  return result->eojeols[eojeol_index].morphemes[morpheme_index];
}

const char     *
hannanum_result_tag(const hannanum_result_t * result, size_t eojeol_index, size_t morpheme_index)
{
  if (result == NULL || eojeol_index >= result->count || morpheme_index >= result->eojeols[eojeol_index].length) {
    return NULL;
  }
  return result->eojeols[eojeol_index].tags[morpheme_index];
}

size_t
hannanum_result_candidate_count(const hannanum_result_t * result, size_t eojeol_index)
{
  if (result == NULL || result->candidate_sets == NULL || eojeol_index >= result->count) {
    return 0;
  }
  return result->candidate_sets[eojeol_index].count;
}

size_t
hannanum_result_candidate_morpheme_count(const hannanum_result_t * result, size_t eojeol_index, size_t candidate_index)
{
  if (result == NULL || result->candidate_sets == NULL || eojeol_index >= result->count || candidate_index >= result->candidate_sets[eojeol_index].count) {
    return 0;
  }
  return result->candidate_sets[eojeol_index].items[candidate_index].length;
}

const char     *
hannanum_result_candidate_morpheme(const hannanum_result_t * result, size_t eojeol_index, size_t candidate_index, size_t morpheme_index)
{
  if (result == NULL || result->candidate_sets == NULL || eojeol_index >= result->count || candidate_index >= result->candidate_sets[eojeol_index].count || morpheme_index >= result->candidate_sets[eojeol_index].items[candidate_index].length) {
    return NULL;
  }
  return result->candidate_sets[eojeol_index].items[candidate_index].morphemes[morpheme_index];
}

const char     *
hannanum_result_candidate_tag(const hannanum_result_t * result, size_t eojeol_index, size_t candidate_index, size_t morpheme_index)
{
  if (result == NULL || result->candidate_sets == NULL || eojeol_index >= result->count || candidate_index >= result->candidate_sets[eojeol_index].count || morpheme_index >= result->candidate_sets[eojeol_index].items[candidate_index].length) {
    return NULL;
  }
  return result->candidate_sets[eojeol_index].items[candidate_index].tags[morpheme_index];
}

char           *
hannanum_result_format(const hannanum_result_t * result)
{
  size_t          i;
  struct strbuffer          out;
  if (result == NULL) {
    return NULL;
  }
  strbuffer_init(&out, 256);
  if (result->candidate_sets != NULL) {
    for (i = 0; i < result->count; i++) {
      size_t j;
      strbuffer_add_str(&out, result->plain[i]);
      strbuffer_add_byte(&out, '\n');
      for (j = 0; j < result->candidate_sets[i].count; j++) {
        size_t k;
        strbuffer_add_byte(&out, '\t');
        for (k = 0; k < result->candidate_sets[i].items[j].length; k++) {
          if (k != 0) {
            strbuffer_add_byte(&out, '+');
          }
          strbuffer_add_str(&out, result->candidate_sets[i].items[j].morphemes[k]);
          strbuffer_add_byte(&out, '/');
          strbuffer_add_str(&out, result->candidate_sets[i].items[j].tags[k]);
        }
        strbuffer_add_byte(&out, '\n');
      }
      strbuffer_add_byte(&out, '\n');
    }
    return (char *)strbuffer_steal(&out);
  }
  for (i = 0; i < result->count; i++) {
    size_t          j;
    strbuffer_add_str(&out, result->plain[i]);
    strbuffer_add_str(&out, "\n\t");
    for (j = 0; j < result->eojeols[i].length; j++) {
      if (j != 0) {
        strbuffer_add_byte(&out, '+');
      }
      strbuffer_add_str(&out, result->eojeols[i].morphemes[j]);
      strbuffer_add_byte(&out, '/');
      strbuffer_add_str(&out, result->eojeols[i].tags[j]);
    }
    strbuffer_add_str(&out, "\n\n");
    if (result->eojeols[i].length == 1 && strcmp(result->eojeols[i].tags[0], "sf") == 0) {
      strbuffer_add_byte(&out, '\n');
    }
  }
  return (char *)strbuffer_steal(&out);
}
