static char
phrase_char_at(const char *tag, int pos)
{
  return tag != NULL && (int)strlen(tag) > pos ? tag[pos] : '\0';
}

static int
tag_starts(const eojeol_t * e, size_t i, const char *prefix)
{
  return e != NULL && i < e->length && starts_with(e->tags[i], prefix);
}

static void
phrase_tag(const eojeol_t * e, char out[3])
{
  const char     *first;
  const char     *last;
  size_t          end;
  out[0] = '.';
  out[1] = '.';
  out[2] = '\0';
  if (e == NULL || e->length == 0 || e->tags[0][0] == '\0') {
    return;
  }
  first = e->tags[0];
  end = e->length - 1;
  last = e->tags[end];
  switch (first[0]) {
  case 'm':
    if (starts_with(first, "ma")) {
      out[0] = (tag_starts(e, 1, "p") || tag_starts(e, 1, "x") || tag_starts(e, 1, "jcp")) ? 'P' : 'A';
    }
    break;
  case 'e':
    if (starts_with(first, "ecc") || starts_with(first, "ecs")) {
      out[0] = 'C';
    }
    break;
  case 'f':
    out[0] = 'N';
    break;
  case 'i':
    out[0] = tag_starts(e, 1, "j") ? 'N' : 'I';
    break;
  case 'n':
    if ((tag_starts(e, 1, "xsv") || tag_starts(e, 1, "xsm") || tag_starts(e, 1, "p")) && !(tag_starts(e, 2, "etn") || tag_starts(e, 3, "etn"))) {
      out[0] = 'P';
    } else {
      out[0] = 'N';
    }
    break;
  case 'p':
    out[0] = tag_starts(e, 1, "xsa") ? 'A' : (tag_starts(e, 1, "etn") ? 'N' : 'P');
    break;
  case 's':
    out[0] = (tag_starts(e, 1, "su") || tag_starts(e, 2, "j") || tag_starts(e, end, "j")) ? 'N' : 'S';
    break;
  case 'x':
    if (starts_with(first, "xsn") || starts_with(first, "xp"))
      out[0] = 'N';
    break;
  default:
    break;
  }
  switch (phrase_char_at(last, 0)) {
  case 'e':
    if (starts_with(last, "ecc") || starts_with(last, "ecs") || starts_with(last, "ecx"))
      out[1] = 'C';
    else if (starts_with(last, "ef"))
      out[1] = 'F';
    else if (starts_with(last, "etm"))
      out[1] = 'M';
    else if (starts_with(last, "etn"))
      out[1] = 'N';
    break;
  case 'j':
    if (starts_with(last, "jcv"))
      out[0] = 'I';
    else if (starts_with(last, "jx"))
      out[1] = out[0] == 'A' ? 'J' : 'X';
    else if (starts_with(last, "jcj"))
      out[1] = out[0] == 'A' ? 'J' : 'Y';
    else if (starts_with(last, "jca"))
      out[1] = 'A';
    else if (starts_with(last, "jcm"))
      out[1] = out[0] == 'A' ? 'J' : 'M';
    else if (starts_with(last, "jc"))
      out[1] = 'J';
    break;
  case 'm':
    if (starts_with(last, "mag"))
      out[1] = 'A';
    break;
  case 'n':
    out[0] = 'N';
    break;
  case 's':
    if (starts_with(last, "sf"))
      out[1] = 'F';
    break;
  case 'x':
    if (starts_with(last, "xsa"))
      out[1] = 'A';
    break;
  default:
    break;
  }
  if (out[0] == out[1]) {
    out[1] = '.';
  } else if (out[0] == '.') {
    out[0] = out[1];
    out[1] = '.';
  }
  if (out[0] == 'A' && out[1] == 'M')
    out[0] = 'N';
  else if (out[0] == 'M' && (out[1] == 'A' || out[1] == 'F' || out[1] == 'C'))
    out[0] = out[1] == 'A' ? 'A' : 'N';
  else if (out[0] == 'I' && (out[1] == 'M' || out[1] == 'J' || out[1] == 'F'))
    out[0] = 'N';
  else if (out[0] == 'I' && out[1] == 'C')
    out[0] = 'P';
  if (out[0] == out[1]) {
    out[1] = '.';
  }
}

static double
prob_or_default(prob_entry_t * *table, const char *key)
{
  double          value;
  return prob_get(table, key, &value) ? value : HANNANUM_DEFAULT_PROB;
}

static double
compute_wt(hannanum_t * h, const eojeol_t * e)
{
  size_t          i;
  double          current;
  char            key[1024];
  const char     *tag;
  double          tbigram;
  double          tunigram;
  double          lexicon;
  if (e == NULL || e->length == 0) {
    return 0.0;
  }
  tag = e->tags[0];
  snprintf(key, sizeof(key), "bnk-%s", tag);
  tbigram = prob_or_default(h->ptt_pos, key);
  tunigram = prob_or_default(h->ptt_pos, tag);
  snprintf(key, sizeof(key), "%s/%s", e->morphemes[0], tag);
  lexicon = prob_or_default(h->pwt, key);
  current = lexicon + tbigram - tunigram;
  for (i = 1; i < e->length; i++) {
    const char     *oldtag = e->tags[i - 1];
    tag = e->tags[i];
    snprintf(key, sizeof(key), "%s-%s", oldtag, tag);
    tbigram = prob_or_default(h->ptt_pos, key);
    snprintf(key, sizeof(key), "%s/%s", e->morphemes[i], tag);
    lexicon = prob_or_default(h->pwt, key);
    tunigram = prob_or_default(h->ptt_pos, tag);
    current += lexicon + tbigram - tunigram;
  }
  snprintf(key, sizeof(key), "%s-bnk", e->tags[e->length - 1]);
  tbigram = prob_or_default(h->ptt_pos, key);
  tunigram = prob_or_default(h->ptt_pos, "bnk");
  current += tbigram - tunigram;
  return current;
}

static double
wp_transition(hannanum_t * h, const char *from, const char *to)
{
  char            key[32];
  double          value;
  double          unigram;
  snprintf(key, sizeof(key), "%s-%s", from, to);
  value = prob_get(h->ptt_wp, key, &value) ? value : HANNANUM_WP_SMOOTHING;
  if (prob_get(h->ptt_wp, to, &unigram)) {
    value -= unigram;
  }
  return value;
}

static int
select_best(hannanum_t * h, candidate_list_t * sets, size_t count, size_t * selected)
{
  hmm_node_t    **nodes;
  size_t         *node_counts;
  size_t          i;
  nodes = (hmm_node_t * *) calloc(count + 1, sizeof(hmm_node_t *));
  node_counts = (size_t *) calloc(count + 1, sizeof(size_t));
  if (nodes == NULL || node_counts == NULL) {
    free(nodes);
    free(node_counts);
    return 0;
  }
  for (i = 0; i < count; i++) {
    size_t          j;
    node_counts[i] = sets[i].count;
    nodes[i] = (hmm_node_t *) calloc(node_counts[i], sizeof(hmm_node_t));
    if (nodes[i] == NULL) {
      free(nodes);
      free(node_counts);
      return 0;
    }
    for (j = 0; j < node_counts[i]; j++) {
      nodes[i][j].eojeol = &sets[i].items[j];
      phrase_tag(&sets[i].items[j], nodes[i][j].phrase);
      nodes[i][j].wt = compute_wt(h, &sets[i].items[j]);
      nodes[i][j].score = i == 0 ? nodes[i][j].wt : -DBL_MAX;
    }
  }
  for (i = 0; i + 1 < count; i++) {
    size_t          j;
    for (j = 0; j < node_counts[i]; j++) {
      size_t          k;
      for (k = 0; k < node_counts[i + 1]; k++) {
        double          score = nodes[i][j].score + wp_transition(h, nodes[i][j].phrase, nodes[i + 1][k].phrase) + nodes[i + 1][k].wt;
        if (!nodes[i + 1][k].has_back || score > nodes[i + 1][k].score) {
          nodes[i + 1][k].score = score;
          nodes[i + 1][k].back = j;
          nodes[i + 1][k].has_back = 1;
        }
      }
    }
  }
  if (count > 0) {
    size_t          best = 0;
    for (i = count; i > 0; i--) {
      size_t          pos = i - 1;
      selected[pos] = best;
      best = nodes[pos][best].back;
    }
  }
  for (i = 0; i < count; i++) {
    free(nodes[i]);
  }
  free(nodes);
  free(node_counts);
  return 1;
}

static int
candidate_list_is_sentence_final(const candidate_list_t *list)
{
  size_t i;
  if (list == NULL || list->count == 0) {
    return 0;
  }
  for (i = 0; i < list->count; i++) {
    size_t j;
    if (list->items[i].length == 0) {
      return 0;
    }
    for (j = 0; j < list->items[i].length; j++) {
      if (strcmp(list->items[i].tags[j], "sf") != 0) {
        return 0;
      }
    }
  }
  return 1;
}

static int
candidate_first_is_single_sentence_final(const candidate_list_t *list)
{
  if (list == NULL || list->count == 0) {
    return 0;
  }
  return list->items[0].length == 1 && strcmp(list->items[0].tags[0], "sf") == 0;
}

static int
select_best_by_sentence(hannanum_t *h, candidate_list_t *sets, size_t count, size_t *selected)
{
  size_t start = 0;
  size_t i;
  while (start < count) {
    size_t end = start;
    while (end < count) {
      if (candidate_list_is_sentence_final(&sets[end])) {
        selected[end] = 0;
        end++;
        break;
      }
      if (candidate_first_is_single_sentence_final(&sets[end])) {
        selected[end] = 0;
        end++;
        break;
      }
      end++;
    }
    if (!select_best(h, sets + start, end - start, selected + start)) {
      return 0;
    }
    start = end;
  }
  for (i = 0; i < count; i++) {
    if (selected[i] >= sets[i].count) {
      return 0;
    }
  }
  return 1;
}
