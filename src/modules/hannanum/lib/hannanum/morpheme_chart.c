#include "morpheme_chart.h"

static void HANNANUM_UNUSED
morpheme_chart_clear(morpheme_chart_t *chart)
{
  int i;
  if (chart == NULL) {
    return;
  }
  for (i = 0; i < HANNANUM_MAX_MORPHEME_CHART; i++) {
    free(chart->nodes[i].str);
    chart->nodes[i].str = NULL;
    chart->nodes[i].connection_count = 0;
    chart->nodes[i].state = HANNANUM_MORPHEME_STATE_FAIL;
  }
  for (i = 0; i < chart->eng_replacement_count; i++) {
    free(chart->eng_replacements[i]);
    chart->eng_replacements[i] = NULL;
  }
  for (i = 0; i < chart->chi_replacement_count; i++) {
    free(chart->chi_replacements[i]);
    chart->chi_replacements[i] = NULL;
  }
  chart->chart_end = 0;
  chart->eng_replacement_count = 0;
  chart->chi_replacement_count = 0;
}

static int HANNANUM_UNUSED
morpheme_chart_init(morpheme_chart_t *chart)
{
  if (chart == NULL) {
    return 0;
  }
  memset(chart, 0, sizeof(*chart));
  return 1;
}

static int
morpheme_chart_is_chinese_codepoint(unsigned int c)
{
  return (c >= 0x2e80 && c <= 0x2eff) || (c >= 0x3400 && c <= 0x4dbf) || (c >= 0x4e00 && c < 0x9fbf) || (c >= 0xf900 && c <= 0xfaff);
}

static char * HANNANUM_UNUSED
morpheme_chart_pre_replace(morpheme_chart_t *chart, const char *input)
{
  const unsigned char *p = (const unsigned char *)input;
  struct strbuffer result;
  struct strbuffer current;
  int eng_flag = 0;
  int chi_flag = 0;
  strbuffer_init(&result, (size_t)strlen(input));
  strbuffer_init(&current, 32);
  if (chart != NULL) {
    chart->eng_replacement_count = 0;
    chart->chi_replacement_count = 0;
  }
  while (*p != '\0') {
    unsigned int c;
    size_t width;
    if (!utf8_decode_one(p, &c, &width)) {
      strbuffer_release(&result);
      strbuffer_release(&current);
      return NULL;
    }
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
      if (!eng_flag) {
        if (chi_flag && chart != NULL) {
          if (chart->chi_replacement_count < 128) {
            chart->chi_replacements[chart->chi_replacement_count++] = (char *)strbuffer_steal(&current);
            strbuffer_init(&current, 32);
          }
        }
        chi_flag = 0;
        strbuffer_reset(&current);
        strbuffer_add_str(&result, "HAN_ENG");
        eng_flag = 1;
      }
      strbuffer_add(&current, p, (size_t)width);
    } else if (morpheme_chart_is_chinese_codepoint(c)) {
      if (!chi_flag) {
        if (eng_flag && chart != NULL) {
          if (chart->eng_replacement_count < 128) {
            chart->eng_replacements[chart->eng_replacement_count++] = (char *)strbuffer_steal(&current);
            strbuffer_init(&current, 32);
          }
        }
        eng_flag = 0;
        strbuffer_reset(&current);
        strbuffer_add_str(&result, "HAN_CHI");
        chi_flag = 1;
      }
      strbuffer_add(&current, p, (size_t)width);
    } else {
      strbuffer_add(&result, p, (size_t)width);
      if (eng_flag && chart != NULL) {
        if (chart->eng_replacement_count < 128) {
          chart->eng_replacements[chart->eng_replacement_count++] = (char *)strbuffer_steal(&current);
          strbuffer_init(&current, 32);
        }
      }
      if (chi_flag && chart != NULL) {
        if (chart->chi_replacement_count < 128) {
          chart->chi_replacements[chart->chi_replacement_count++] = (char *)strbuffer_steal(&current);
          strbuffer_init(&current, 32);
        }
      }
      eng_flag = 0;
      chi_flag = 0;
      strbuffer_reset(&current);
    }
    p += width;
  }
  if (eng_flag && chart != NULL) {
    if (chart->eng_replacement_count < 128) {
      chart->eng_replacements[chart->eng_replacement_count++] = (char *)strbuffer_steal(&current);
      strbuffer_init(&current, 32);
    }
  }
  if (chi_flag && chart != NULL) {
    if (chart->chi_replacement_count < 128) {
      chart->chi_replacements[chart->chi_replacement_count++] = (char *)strbuffer_steal(&current);
      strbuffer_init(&current, 32);
    }
  }
  strbuffer_release(&current);
  return (char *)strbuffer_steal(&result);
}

static int HANNANUM_UNUSED
morpheme_chart_init_word(morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, const char *word, int iwg_tag)
{
  char *replaced;
  codepoint_vec_t triple;
  segment_position_node_t *start;
  int start_node;
  if (chart == NULL || sp == NULL || simti == NULL || word == NULL) {
    return 0;
  }
  morpheme_chart_clear(chart);
  replaced = morpheme_chart_pre_replace(chart, word);
  if (replaced == NULL) {
    return 0;
  }
  memset(&triple, 0, sizeof(triple));
  if (!hannanum_code_to_triple(replaced, &triple)) {
    free(replaced);
    return 0;
  }
  free(replaced);
  if (!segment_position_init_from_triple_with_simti(sp, &triple, simti)) {
    codepoint_vec_free(&triple);
    return 0;
  }
  codepoint_vec_free(&triple);
  start = segment_position_get(sp, 0);
  if (start == NULL) {
    return 0;
  }
  start_node = morpheme_chart_add(chart, iwg_tag, 0, 1, 0, "");
  if (start_node != 0) {
    return 0;
  }
  chart->nodes[start_node].state = HANNANUM_MORPHEME_STATE_SUCCESS;
  start->morpheme[start->morph_count++] = start_node;
  return 1;
}

static int HANNANUM_UNUSED
morpheme_chart_add(morpheme_chart_t *chart, int tag, int phoneme, int next_position, int next_tag_type, const char *str)
{
  morpheme_chart_node_t *node;
  if (chart == NULL || chart->chart_end >= HANNANUM_MAX_MORPHEME_CHART) {
    return -1;
  }
  node = &chart->nodes[chart->chart_end];
  node->tag = tag;
  node->phoneme = phoneme;
  node->next_position = next_position;
  node->next_tag_type = next_tag_type;
  node->state = HANNANUM_MORPHEME_STATE_INCOMPLETE;
  node->connection_count = 0;
  node->str = hn_strdup(str != NULL ? str : "");
  if (node->str == NULL) {
    return -1;
  }
  return chart->chart_end++;
}

static int HANNANUM_UNUSED
morpheme_chart_add_connection(morpheme_chart_t *chart, int from, int to)
{
  morpheme_chart_node_t *node;
  if (chart == NULL || from < 0 || from >= chart->chart_end || to < 0 || to >= chart->chart_end) {
    return 0;
  }
  node = &chart->nodes[from];
  if (node->connection_count >= HANNANUM_MAX_MORPHEME_CONNECTION) {
    return 0;
  }
  node->connection[node->connection_count++] = to;
  return 1;
}

static int HANNANUM_UNUSED
morpheme_chart_check(const morpheme_chart_t *chart, const int *morphemes, int morpheme_len, int tag, int phoneme, int next_position, int next_tag_type, const char *str)
{
  int i;
  if (chart == NULL || morphemes == NULL || str == NULL) {
    return 0;
  }
  for (i = 0; i < morpheme_len; i++) {
    int index = morphemes[i];
    const morpheme_chart_node_t *node;
    if (index < 0 || index >= chart->chart_end) {
      continue;
    }
    node = &chart->nodes[index];
    if (node->tag == tag && node->phoneme == phoneme && node->next_position == next_position && node->next_tag_type == next_tag_type && strcmp(node->str, str) == 0) {
      return 1;
    }
  }
  return 0;
}

static int HANNANUM_UNUSED
morpheme_chart_node_to_eojeol(const morpheme_chart_t *chart, const int *path, size_t path_len, eojeol_t *out, char **tag_names, size_t tag_count)
{
  size_t i;
  int eng_index = 0;
  int chi_index = 0;
  if (chart == NULL || path == NULL || out == NULL) {
    return 0;
  }
  memset(out, 0, sizeof(*out));
  out->morphemes = (char **)calloc(path_len, sizeof(char *));
  out->tags = (char **)calloc(path_len, sizeof(char *));
  if (out->morphemes == NULL || out->tags == NULL) {
    free(out->morphemes);
    free(out->tags);
    memset(out, 0, sizeof(*out));
    return 0;
  }
  out->length = path_len;
  for (i = 0; i < path_len; i++) {
    int index = path[i];
    const morpheme_chart_node_t *node;
    if (index < 0 || index >= chart->chart_end) {
      free_eojeol(out);
      return 0;
    }
    node = &chart->nodes[index];
    if (strstr(node->str, "HAN_ENG") != NULL || strstr(node->str, "HAN_CHI") != NULL) {
      const char *p = node->str;
      struct strbuffer restored;
      strbuffer_init(&restored, (size_t)strlen(node->str));
      while (*p != '\0') {
        if (strncmp(p, "HAN_ENG", 7) == 0 && eng_index < chart->eng_replacement_count) {
          strbuffer_add_str(&restored, chart->eng_replacements[eng_index++]);
          p += 7;
        } else if (strncmp(p, "HAN_CHI", 7) == 0 && chi_index < chart->chi_replacement_count) {
          strbuffer_add_str(&restored, chart->chi_replacements[chi_index++]);
          p += 7;
        } else {
          strbuffer_add_byte(&restored, (unsigned char)*p++);
        }
      }
      out->morphemes[i] = (char *)strbuffer_steal(&restored);
    } else {
      out->morphemes[i] = hn_strdup(node->str);
    }
    if (node->tag >= 0 && (size_t)node->tag < tag_count) {
      out->tags[i] = hn_strdup(tag_names[node->tag]);
    } else {
      out->tags[i] = hn_strdup("unk");
    }
    if (out->morphemes[i] == NULL || out->tags[i] == NULL) {
      free_eojeol(out);
      return 0;
    }
  }
  return 1;
}

static int HANNANUM_UNUSED
morpheme_chart_alt_segment(segment_position_t *sp, simti_t *simti, const codepoint_vec_t *segment)
{
  unsigned int reverse_word[1024];
  size_t match;
  size_t len;
  size_t i;
  int previous = 0;
  int to;
  if (sp == NULL || simti == NULL || segment == NULL || segment->count == 0 || segment->count > 1024) {
    return 0;
  }
  for (i = 0; i < segment->count; i++) {
    reverse_word[i] = segment->items[segment->count - 1 - i];
  }
  match = simti_search(simti, reverse_word, segment->count);
  to = match == 0 ? 0 : simti_fetch(simti, reverse_word, match);
  len = segment->count;
  for (i = 0; i < segment->count; i++) {
    int next;
    if (len <= match) {
      break;
    }
    next = segment_position_add(sp, segment->items[i]);
    if (next == 0) {
      return 0;
    }
    if (previous != 0) {
      segment_position_set_link(sp, previous, next);
    }
    simti_insert(simti, reverse_word, len, next);
    previous = next;
    len--;
  }
  if (previous != 0) {
    segment_position_set_link(sp, previous, to);
  }
  return simti_fetch(simti, reverse_word, segment->count);
}

static int HANNANUM_UNUSED
morpheme_chart_phoneme_change(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int from, const codepoint_vec_t *front, const codepoint_vec_t *back, int front_tag_type, int back_tag_type, int phoneme)
{
  trie_node_t *node;
  trie_info_t *info;
  segment_position_node_t *position;
  char *front_text;
  int added = 0;
  if (h == NULL || chart == NULL || sp == NULL || simti == NULL || front == NULL || back == NULL || from < 0) {
    return 0;
  }
  node = trie_search_codepoints(h->system_trie, front->items, front->count);
  if (node == NULL || node->info == NULL) {
    return 0;
  }
  position = segment_position_get(sp, from);
  if (position == NULL) {
    return 0;
  }
  front_text = hannanum_code_from_triple(front);
  if (front_text == NULL) {
    return 0;
  }
  for (info = node->info; info != NULL; info = info->next) {
    int next;
    int chart_index;
    if (!check_tag_type(h, front_tag_type, info->tag) || !check_phoneme_type(phoneme, info->phoneme)) {
      continue;
    }
    next = morpheme_chart_alt_segment(sp, simti, back);
    if (next == 0 && back->count != 0) {
      continue;
    }
    if (morpheme_chart_check(chart, position->morpheme, position->morph_count, info->tag, info->phoneme, next, back_tag_type, front_text)) {
      continue;
    }
    if (position->morph_count >= HANNANUM_MAX_MORPHEME_COUNT) {
      break;
    }
    chart_index = morpheme_chart_add(chart, info->tag, info->phoneme, next, back_tag_type, front_text);
    if (chart_index < 0) {
      break;
    }
    position->morpheme[position->morph_count++] = chart_index;
    added++;
  }
  free(front_text);
  return added;
}

static int HANNANUM_UNUSED
morpheme_chart_scan_one_trie(morpheme_chart_t *chart, segment_position_t *sp, segment_position_node_t *from_pos, hannanum_trie_t *trie, int from)
{
  trie_node_t *node;
  int to;
  int segment_path[HANNANUM_MAX_SEGMENT];
  int segment_len = 0;
  int added = 0;
  codepoint_vec_t prefix;
  struct {
    trie_node_t *node;
    int next_position;
    char *text;
  } matches[HANNANUM_MAX_SEGMENT];
  int match_count = 0;
  if (chart == NULL || sp == NULL || from_pos == NULL || trie == NULL) {
    return 0;
  }
  memset(&prefix, 0, sizeof(prefix));
  node = &trie->root;
  for (to = from; to != (int)HANNANUM_POSITION_START_KEY; to = segment_position_next(sp, to)) {
    segment_position_node_t *to_pos = segment_position_get(sp, to);
    if (to_pos == NULL || segment_len >= HANNANUM_MAX_SEGMENT) {
      break;
    }
    node = trie_node_look_node(node, to_pos->key);
    if (node == NULL) {
      break;
    }
    if (!codepoint_vec_push(&prefix, to_pos->key)) {
      break;
    }
    segment_path[segment_len++] = to;
    if (node->info != NULL) {
      char *text = hannanum_code_from_triple(&prefix);
      if (text == NULL) {
        break;
      }
      if (match_count < HANNANUM_MAX_SEGMENT) {
        matches[match_count].node = node;
        matches[match_count].next_position = segment_position_next(sp, to);
        matches[match_count].text = text;
        match_count++;
      } else {
        free(text);
      }
    }
  }
  while (match_count > 0) {
    trie_info_t *info;
    match_count--;
    for (info = matches[match_count].node->info; info != NULL; info = info->next) {
      int chart_index;
      if (morpheme_chart_check(chart, from_pos->morpheme, from_pos->morph_count, info->tag, info->phoneme, matches[match_count].next_position, 0, matches[match_count].text)) {
        continue;
      }
      if (from_pos->morph_count >= HANNANUM_MAX_MORPHEME_COUNT) {
        break;
      }
      chart_index = morpheme_chart_add(chart, info->tag, info->phoneme, matches[match_count].next_position, 0, matches[match_count].text);
      if (chart_index < 0) {
        break;
      }
      from_pos->morpheme[from_pos->morph_count++] = chart_index;
      added++;
    }
    free(matches[match_count].text);
  }
  (void)segment_path;
  codepoint_vec_free(&prefix);
  return added;
}

static void
morpheme_chart_add_trie_match(morpheme_chart_t *chart, segment_position_node_t *from_pos, trie_node_t *node, int next_position, const char *text, int *added)
{
  trie_info_t *info;
  for (info = node->info; info != NULL; info = info->next) {
    int chart_index;
    if (morpheme_chart_check(chart, from_pos->morpheme, from_pos->morph_count, info->tag, info->phoneme, next_position, 0, text)) {
      continue;
    }
    if (from_pos->morph_count >= HANNANUM_MAX_MORPHEME_COUNT) {
      break;
    }
    chart_index = morpheme_chart_add(chart, info->tag, info->phoneme, next_position, 0, text);
    if (chart_index < 0) {
      break;
    }
    from_pos->morpheme[from_pos->morph_count++] = chart_index;
    (*added)++;
  }
}

static int HANNANUM_UNUSED
morpheme_chart_scan_system_user_tries(morpheme_chart_t *chart, segment_position_t *sp, segment_position_node_t *from_pos, hannanum_trie_t *system_trie, hannanum_trie_t *user_trie, int from)
{
  trie_node_t *system_node;
  trie_node_t *user_node;
  int to;
  int added = 0;
  codepoint_vec_t prefix;
  struct {
    trie_node_t *system_node;
    trie_node_t *user_node;
    int next_position;
    char *text;
  } matches[HANNANUM_MAX_SEGMENT];
  int match_count = 0;
  if (chart == NULL || sp == NULL || from_pos == NULL || system_trie == NULL || user_trie == NULL) {
    return 0;
  }
  memset(&prefix, 0, sizeof(prefix));
  system_node = &system_trie->root;
  user_node = &user_trie->root;
  for (to = from; to != (int)HANNANUM_POSITION_START_KEY; to = segment_position_next(sp, to)) {
    segment_position_node_t *to_pos = segment_position_get(sp, to);
    if (to_pos == NULL || match_count >= HANNANUM_MAX_SEGMENT) {
      break;
    }
    if (system_node != NULL) {
      system_node = trie_node_look_node(system_node, to_pos->key);
    }
    if (user_node != NULL) {
      user_node = trie_node_look_node(user_node, to_pos->key);
    }
    if (system_node == NULL && user_node == NULL) {
      break;
    }
    if (!codepoint_vec_push(&prefix, to_pos->key)) {
      break;
    }
    if ((system_node != NULL && system_node->info != NULL) || (user_node != NULL && user_node->info != NULL)) {
      char *text = hannanum_code_from_triple(&prefix);
      if (text == NULL) {
        break;
      }
      matches[match_count].system_node = system_node;
      matches[match_count].user_node = user_node;
      matches[match_count].next_position = segment_position_next(sp, to);
      matches[match_count].text = text;
      match_count++;
    }
  }
  while (match_count > 0) {
    match_count--;
    if (matches[match_count].system_node != NULL && matches[match_count].system_node->info != NULL) {
      morpheme_chart_add_trie_match(chart, from_pos, matches[match_count].system_node, matches[match_count].next_position, matches[match_count].text, &added);
    }
    if (matches[match_count].user_node != NULL && matches[match_count].user_node->info != NULL) {
      morpheme_chart_add_trie_match(chart, from_pos, matches[match_count].user_node, matches[match_count].next_position, matches[match_count].text, &added);
    }
    free(matches[match_count].text);
  }
  codepoint_vec_free(&prefix);
  return added;
}

static int HANNANUM_UNUSED
morpheme_chart_scan_dictionaries(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int chart_index)
{
  morpheme_chart_node_t *morph;
  segment_position_node_t *from_pos;
  int from;
  int added = 0;
  if (h == NULL || chart == NULL || sp == NULL || chart_index < 0 || chart_index >= chart->chart_end) {
    return 0;
  }
  morph = &chart->nodes[chart_index];
  from = morph->next_position;
  from_pos = segment_position_get(sp, from);
  if (from_pos == NULL || from_pos->state != HANNANUM_SP_STATE_N) {
    return 0;
  }
  {
    int state = 1;
    int to;
    codepoint_vec_t prefix;
    int num_tag = tag_id(h, "nnc");
    int best_next_position = -1;
    char *best_text = NULL;
    memset(&prefix, 0, sizeof(prefix));
    for (to = from; to != (int)HANNANUM_POSITION_START_KEY; to = segment_position_next(sp, to)) {
      segment_position_node_t *to_pos = segment_position_get(sp, to);
      if (to_pos == NULL) {
        break;
      }
      state = number_dic_node_look(to_pos->key, state);
      if (state == 0) {
        break;
      }
      if (!codepoint_vec_push(&prefix, to_pos->key)) {
        break;
      }
      to_pos->n_index = state;
      if (number_dic_is_num(state) && num_tag >= 0) {
        char *text = hannanum_code_from_triple(&prefix);
        if (text == NULL) {
          break;
        }
        free(best_text);
        best_text = text;
        best_next_position = segment_position_next(sp, to);
      }
    }
    if (best_text != NULL && best_next_position >= 0 && !morpheme_chart_check(chart, from_pos->morpheme, from_pos->morph_count, num_tag, 0, best_next_position, 0, best_text) && from_pos->morph_count < HANNANUM_MAX_MORPHEME_COUNT) {
      int chart_node = morpheme_chart_add(chart, num_tag, 0, best_next_position, 0, best_text);
      if (chart_node >= 0) {
        from_pos->morpheme[from_pos->morph_count++] = chart_node;
        added++;
      }
    }
    free(best_text);
    codepoint_vec_free(&prefix);
  }
  added += morpheme_chart_scan_system_user_tries(chart, sp, from_pos, h->system_trie, h->user_trie, from);
  from_pos->state = HANNANUM_SP_STATE_D;
  return added;
}

static int HANNANUM_UNUSED
morpheme_chart_expand_d_state(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int chart_index, morpheme_chart_expander_t expander, void *userdata)
{
  morpheme_chart_node_t *morph;
  segment_position_node_t *from_pos;
  int from;
  int added;
  if (h == NULL || chart == NULL || sp == NULL || simti == NULL || expander == NULL || chart_index < 0 || chart_index >= chart->chart_end) {
    return 0;
  }
  morph = &chart->nodes[chart_index];
  from = morph->next_position;
  from_pos = segment_position_get(sp, from);
  if (from_pos == NULL || from_pos->state != HANNANUM_SP_STATE_D) {
    return 0;
  }
  added = expander(h, chart, sp, simti, from, morph->str, userdata);
  from_pos->state = HANNANUM_SP_STATE_R;
  return added;
}

static int HANNANUM_UNUSED
morpheme_chart_connect_m_state(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int chart_index, morpheme_chart_connector_t connector, void *userdata)
{
  morpheme_chart_node_t *morph;
  segment_position_node_t *from_pos;
  int i;
  int added = 0;
  if (h == NULL || chart == NULL || sp == NULL || connector == NULL || chart_index < 0 || chart_index >= chart->chart_end) {
    return 0;
  }
  morph = &chart->nodes[chart_index];
  from_pos = segment_position_get(sp, morph->next_position);
  if (from_pos == NULL || (from_pos->state != HANNANUM_SP_STATE_M && from_pos->state != HANNANUM_SP_STATE_R)) {
    return 0;
  }
  for (i = 0; i < from_pos->morph_count; i++) {
    int next = from_pos->morpheme[i];
    if (next < 0 || next >= chart->chart_end) {
      continue;
    }
    if (chart->nodes[next].state == HANNANUM_MORPHEME_STATE_SUCCESS && connector(h, morph, &chart->nodes[next], userdata)) {
      if (morpheme_chart_add_connection(chart, chart_index, next)) {
        added++;
      }
    }
  }
  return added;
}

static int HANNANUM_UNUSED
morpheme_chart_process_r_state(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int chart_index, int tag_type, morpheme_chart_tag_type_t tag_type_check, morpheme_chart_recursor_t recursor, void *userdata)
{
  morpheme_chart_node_t *morph;
  segment_position_node_t *from_pos;
  int i;
  int x = 0;
  if (h == NULL || chart == NULL || sp == NULL || tag_type_check == NULL || recursor == NULL || chart_index < 0 || chart_index >= chart->chart_end) {
    return 0;
  }
  morph = &chart->nodes[chart_index];
  from_pos = segment_position_get(sp, morph->next_position);
  if (from_pos == NULL || from_pos->state != HANNANUM_SP_STATE_R) {
    return 0;
  }
  for (i = 0; i < from_pos->morph_count; i++) {
    int mp = from_pos->morpheme[i];
    int y;
    if (mp < 0 || mp >= chart->chart_end) {
      continue;
    }
    if (!tag_type_check(h, tag_type, chart->nodes[mp].tag, userdata)) {
      continue;
    }
    if (chart->nodes[mp].state == HANNANUM_MORPHEME_STATE_INCOMPLETE) {
      y = recursor(h, chart, sp, mp, chart->nodes[mp].next_tag_type, userdata);
      x += y;
      chart->nodes[mp].state = y != 0 ? HANNANUM_MORPHEME_STATE_SUCCESS : HANNANUM_MORPHEME_STATE_FAIL;
    } else {
      x += chart->nodes[mp].connection_count;
    }
  }
  if (x == 0) {
    if (tag_type == HANNANUM_TAG_TYPE_ALL) {
      from_pos->state = HANNANUM_SP_STATE_F;
    }
    return 0;
  }
  if (tag_type == HANNANUM_TAG_TYPE_ALL) {
    from_pos->state = HANNANUM_SP_STATE_M;
  }
  return x;
}

typedef struct morpheme_chart_analyze_context {
  simti_t *simti;
  morpheme_chart_expander_t expander;
  morpheme_chart_tag_type_t tag_type_check;
  morpheme_chart_connector_t connector;
  void *userdata;
} morpheme_chart_analyze_context_t;

static int
morpheme_chart_recurse_adapter(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int chart_index, int next_tag_type, void *userdata)
{
  morpheme_chart_analyze_context_t *ctx = (morpheme_chart_analyze_context_t *)userdata;
  return morpheme_chart_analyze_with_callbacks(h, chart, sp, ctx->simti, chart_index, next_tag_type, ctx->expander, ctx->tag_type_check, ctx->connector, ctx->userdata);
}

static int HANNANUM_UNUSED
morpheme_chart_analyze_with_callbacks(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int chart_index, int tag_type, morpheme_chart_expander_t expander, morpheme_chart_tag_type_t tag_type_check, morpheme_chart_connector_t connector, void *userdata)
{
  morpheme_chart_node_t *morph;
  segment_position_node_t *from_pos;
  morpheme_chart_analyze_context_t ctx;
  int from;
  if (h == NULL || chart == NULL || sp == NULL || simti == NULL || expander == NULL || tag_type_check == NULL || connector == NULL || chart_index < 0 || chart_index >= chart->chart_end) {
    return 0;
  }
  morph = &chart->nodes[chart_index];
  from = morph->next_position;
  from_pos = segment_position_get(sp, from);
  if (from_pos == NULL) {
    return 0;
  }
  if (from_pos->state == HANNANUM_SP_STATE_N) {
    morpheme_chart_scan_dictionaries(h, chart, sp, chart_index);
  }
  if (from_pos->state == HANNANUM_SP_STATE_D) {
    morpheme_chart_expand_d_state(h, chart, sp, simti, chart_index, expander, userdata);
  }
  if (from_pos->state == HANNANUM_SP_STATE_R) {
    ctx.simti = simti;
    ctx.expander = expander;
    ctx.tag_type_check = tag_type_check;
    ctx.connector = connector;
    ctx.userdata = userdata;
    morpheme_chart_process_r_state(h, chart, sp, chart_index, tag_type, tag_type_check, morpheme_chart_recurse_adapter, &ctx);
  }
  if (from_pos->state == HANNANUM_SP_STATE_M || from_pos->state == HANNANUM_SP_STATE_R) {
    morpheme_chart_connect_m_state(h, chart, sp, chart_index, connector, userdata);
  }
  if (from_pos->state == HANNANUM_SP_STATE_F && morph->connection_count == 0) {
    return 0;
  }
  return morph->connection_count;
}

static int
morpheme_chart_collect_results_rec(const morpheme_chart_t *chart, int chart_index, int *path, size_t path_len, char **tag_names, size_t tag_count, candidate_list_t *out)
{
  const morpheme_chart_node_t *node;
  int i;
  if (chart == NULL || out == NULL || chart_index < 0 || chart_index >= chart->chart_end) {
    return 0;
  }
  node = &chart->nodes[chart_index];
  if (chart_index != 0) {
    path[path_len++] = chart_index;
  }
  for (i = 0; i < node->connection_count; i++) {
    int next = node->connection[i];
    if (next == 0) {
      eojeol_t eojeol;
      if (!morpheme_chart_node_to_eojeol(chart, path, path_len, &eojeol, tag_names, tag_count)) {
        return 0;
      }
      if (!candidate_list_add(out, eojeol)) {
        free_eojeol(&eojeol);
        return 0;
      }
    } else if (!morpheme_chart_collect_results_rec(chart, next, path, path_len, tag_names, tag_count, out)) {
      return 0;
    }
  }
  return 1;
}

static int HANNANUM_UNUSED
morpheme_chart_collect_results(const morpheme_chart_t *chart, int chart_index, char **tag_names, size_t tag_count, candidate_list_t *out)
{
  int path[HANNANUM_MAX_MORPHEME_CHART];
  if (out == NULL) {
    return 0;
  }
  return morpheme_chart_collect_results_rec(chart, chart_index, path, 0, tag_names, tag_count, out);
}

static int HANNANUM_UNUSED
morpheme_chart_analyze_unknown(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, int unk_tag)
{
  int i;
  codepoint_vec_t prefix;
  segment_position_node_t *pos_1;
  int added = 0;
  (void)h;
  if (chart == NULL || sp == NULL || unk_tag < 0) {
    return 0;
  }
  pos_1 = segment_position_get(sp, 1);
  if (pos_1 == NULL) {
    return 0;
  }
  memset(&prefix, 0, sizeof(prefix));
  for (i = 1; i != 0; i = segment_position_next(sp, i)) {
    segment_position_node_t *pos = segment_position_get(sp, i);
    char *text;
    int chart_node;
    if (pos == NULL) {
      break;
    }
    if (!codepoint_vec_push(&prefix, pos->key)) {
      break;
    }
    text = hannanum_code_from_triple(&prefix);
    if (text == NULL) {
      break;
    }
    chart_node = morpheme_chart_add(chart, unk_tag, 0, segment_position_next(sp, i), HANNANUM_TAG_TYPE_ALL, text);
    free(text);
    if (chart_node >= 0 && pos_1->morph_count < HANNANUM_MAX_MORPHEME_COUNT) {
      pos_1->morpheme[pos_1->morph_count++] = chart_node;
      pos_1->state = HANNANUM_SP_STATE_R;
      added++;
    }
  }
  chart->nodes[0].connection_count = 0;
  codepoint_vec_free(&prefix);
  return added;
}
