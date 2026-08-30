static dict_entry_t * dict_find(dict_entry_t * *dict, const char *key){
  dict_entry_t   *entry;
  if (dict == NULL || key == NULL) {
    return NULL;
  }
  entry = dict[hash_string(key) % HANNANUM_HASH_SIZE];
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      return entry;
    }
    entry = entry->next;
  }
  return NULL;
}

static dict_entry_t * dict_get_or_create(hannanum_t * h, const char *key){
  unsigned long   idx;
  dict_entry_t   *entry;
  entry = dict_find(h->dict, key);
  if (entry != NULL) {
    return entry;
  }
  entry = (dict_entry_t *) calloc(1, sizeof(dict_entry_t));
  if (entry == NULL) {
    return NULL;
  }
  entry->key = hn_strdup(key);
  if (entry->key == NULL) {
    free(entry);
    return NULL;
  }
  idx = hash_string(key) % HANNANUM_HASH_SIZE;
  entry->next = h->dict[idx];
  h->dict[idx] = entry;
  return entry;
}

static void
prob_put(prob_entry_t * *table, const char *key, double value)
{
  unsigned long   idx;
  prob_entry_t   *entry;
  if (table == NULL || key == NULL || *key == '\0') {
    return;
  }
  idx = hash_string(key) % HANNANUM_HASH_SIZE;
  entry = table[idx];
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      entry->value = value;
      return;
    }
    entry = entry->next;
  }
  entry = (prob_entry_t *) calloc(1, sizeof(prob_entry_t));
  if (entry == NULL) {
    return;
  }
  entry->key = hn_strdup(key);
  if (entry->key == NULL) {
    free(entry);
    return;
  }
  entry->value = value;
  entry->next = table[idx];
  table[idx] = entry;
}

static int
prob_get(prob_entry_t * *table, const char *key, double *value)
{
  prob_entry_t   *entry;
  if (table == NULL || key == NULL) {
    return 0;
  }
  entry = table[hash_string(key) % HANNANUM_HASH_SIZE];
  while (entry != NULL) {
    if (strcmp(entry->key, key) == 0) {
      if (value != NULL) {
        *value = entry->value;
      }
      return 1;
    }
    entry = entry->next;
  }
  return 0;
}

static int
parse_analysis(const char *analysis, eojeol_t * out)
{
  char           *copy;
  char           *save = NULL;
  char           *part;
  size_t          count = 0;
  size_t          i = 0;
  if (analysis == NULL || out == NULL) {
    return 0;
  }
  memset(out, 0, sizeof(*out));
  copy = hn_strdup(analysis);
  if (copy == NULL) {
    return 0;
  }
  for (part = strtok_r(copy, "+", &save); part != NULL; part = strtok_r(NULL, "+", &save)) {
    count++;
  }
  free(copy);
  if (count == 0) {
    return 0;
  }
  out->morphemes = (char **)calloc(count, sizeof(char *));
  out->tags = (char **)calloc(count, sizeof(char *));
  if (out->morphemes == NULL || out->tags == NULL) {
    free(out->morphemes);
    free(out->tags);
    memset(out, 0, sizeof(*out));
    return 0;
  }
  out->length = count;
  copy = hn_strdup(analysis);
  if (copy == NULL) {
    free_eojeol(out);
    return 0;
  }
  save = NULL;
  for (part = strtok_r(copy, "+", &save); part != NULL; part = strtok_r(NULL, "+", &save)) {
    char           *slash = strrchr(part, '/');
    if (slash == NULL || slash == part || slash[1] == '\0') {
      free(copy);
      free_eojeol(out);
      return 0;
    }
    *slash = '\0';
    out->morphemes[i] = hn_strdup(part);
    out->tags[i] = hn_strdup(slash + 1);
    if (out->morphemes[i] == NULL || out->tags[i] == NULL) {
      free(copy);
      free_eojeol(out);
      return 0;
    }
    i++;
  }
  free(copy);
  return 1;
}

static eojeol_t make_single(const char *surface, const char *tag){
  eojeol_t        e;
  memset(&e, 0, sizeof(e));
  e.morphemes = (char **)calloc(1, sizeof(char *));
  e.tags = (char **)calloc(1, sizeof(char *));
  if (e.morphemes == NULL || e.tags == NULL) {
    free(e.morphemes);
    free(e.tags);
    memset(&e, 0, sizeof(e));
    return e;
  }
  e.morphemes[0] = hn_strdup(surface);
  e.tags[0] = hn_strdup(tag);
  if (e.morphemes[0] == NULL || e.tags[0] == NULL) {
    free_eojeol(&e);
    return e;
  }
  e.length = 1;
  return e;
}

static eojeol_t make_pair(const char *first_surface, const char *first_tag, const char *second_surface, const char *second_tag){
  eojeol_t        e;
  memset(&e, 0, sizeof(e));
  e.morphemes = (char **)calloc(2, sizeof(char *));
  e.tags = (char **)calloc(2, sizeof(char *));
  if (e.morphemes == NULL || e.tags == NULL) {
    free(e.morphemes);
    free(e.tags);
    memset(&e, 0, sizeof(e));
    return e;
  }
  e.morphemes[0] = hn_strdup(first_surface);
  e.tags[0] = hn_strdup(first_tag);
  e.morphemes[1] = hn_strdup(second_surface);
  e.tags[1] = hn_strdup(second_tag);
  if (e.morphemes[0] == NULL || e.tags[0] == NULL || e.morphemes[1] == NULL || e.tags[1] == NULL) {
    free_eojeol(&e);
    return e;
  }
  e.length = 2;
  return e;
}

static eojeol_t
make_parts(size_t count, const char **surfaces, const char **tags)
{
  eojeol_t        e;
  size_t          i;
  memset(&e, 0, sizeof(e));
  e.morphemes = (char **)calloc(count, sizeof(char *));
  e.tags = (char **)calloc(count, sizeof(char *));
  if (e.morphemes == NULL || e.tags == NULL) {
    free(e.morphemes);
    free(e.tags);
    memset(&e, 0, sizeof(e));
    return e;
  }
  e.length = count;
  for (i = 0; i < count; i++) {
    e.morphemes[i] = hn_strdup(surfaces[i]);
    e.tags[i] = hn_strdup(tags[i]);
    if (e.morphemes[i] == NULL || e.tags[i] == NULL) {
      free_eojeol(&e);
      return e;
    }
  }
  return e;
}

static int
is_utf8_continuation(unsigned char c)
{
  return (c & 0xc0u) == 0x80u;
}
static int
segment_stack_push(segment_stack_t * stack, const char *morpheme, const char *tag)
{
  char          **next_m;
  char          **next_t;
  if (stack->count == stack->capacity) {
    size_t          new_capacity = stack->capacity == 0 ? 4 : stack->capacity * 2;
    next_m = (char **)realloc(stack->morphemes, new_capacity * sizeof(char *));
    if (next_m == NULL) {
      return 0;
    }
    stack->morphemes = next_m;
    next_t = (char **)realloc(stack->tags, new_capacity * sizeof(char *));
    if (next_t == NULL) {
      return 0;
    }
    stack->tags = next_t;
    stack->capacity = new_capacity;
  }
  stack->morphemes[stack->count] = hn_strdup(morpheme);
  stack->tags[stack->count] = hn_strdup(tag);
  if (stack->morphemes[stack->count] == NULL || stack->tags[stack->count] == NULL) {
    free(stack->morphemes[stack->count]);
    free(stack->tags[stack->count]);
    return 0;
  }
  stack->count++;
  return 1;
}

static void
segment_stack_pop(segment_stack_t * stack)
{
  if (stack == NULL || stack->count == 0) {
    return;
  }
  stack->count--;
  free(stack->morphemes[stack->count]);
  free(stack->tags[stack->count]);
}

static void
segment_stack_free(segment_stack_t * stack)
{
  while (stack != NULL && stack->count > 0) {
    segment_stack_pop(stack);
  }
  if (stack != NULL) {
    free(stack->morphemes);
    free(stack->tags);
    stack->morphemes = NULL;
    stack->tags = NULL;
    stack->capacity = 0;
  }
}

static eojeol_t eojeol_from_stack(const segment_stack_t * stack){
  eojeol_t        e;
  size_t          i;
  memset(&e, 0, sizeof(e));
  if (stack == NULL || stack->count == 0) {
    return e;
  }
  e.morphemes = (char **)calloc(stack->count, sizeof(char *));
  e.tags = (char **)calloc(stack->count, sizeof(char *));
  if (e.morphemes == NULL || e.tags == NULL) {
    free(e.morphemes);
    free(e.tags);
    memset(&e, 0, sizeof(e));
    return e;
  }
  e.length = stack->count;
  for (i = 0; i < stack->count; i++) {
    e.morphemes[i] = hn_strdup(stack->morphemes[i]);
    e.tags[i] = hn_strdup(stack->tags[i]);
    if (e.morphemes[i] == NULL || e.tags[i] == NULL) {
      free_eojeol(&e);
      break;
    }
  }
  return e;
}

static int
segment_tag_allowed(const char *tag, int at_start, int at_end)
{
  if (tag == NULL || *tag == '\0') {
    return 0;
  }
  if (!at_start && (tag[0] == 'p' || tag[0] == 'n' || tag[0] == 'm' || tag[0] == 's' || tag[0] == 'f' || tag[0] == 'j' || tag[0] == 'e' || tag[0] == 'x')) {
    return 1;
  }
  if (at_start && (tag[0] == 'n' || tag[0] == 'p' || tag[0] == 'm' || tag[0] == 's' || tag[0] == 'f')) {
    return 1;
  }
  if (at_end && (tag[0] == 'j' || tag[0] == 'e' || tag[0] == 'x')) {
    return 1;
  }
  return at_start && at_end;
}

static int
segment_connection_ok(hannanum_t *h, const char *prev, const char *next)
{
  if (prev == NULL || next == NULL) {
    return 1;
  }
  if ((prev[0] == 'n' || prev[0] == 'm' || prev[0] == 's' || prev[0] == 'f') && (next[0] == 'j' || next[0] == 'x')) {
    return 1;
  }
  if (prev[0] == 'p' && (next[0] == 'e' || next[0] == 'x')) {
    return 1;
  }
  if (prev[0] == 'e' && next[0] == 'j') {
    return 1;
  }
  if (prev[0] == 'j' && next[0] == 'j') {
    return 1;
  }
  if (prev[0] == 'x' && (next[0] == 'e' || next[0] == 'j')) {
    return 1;
  }
  return connection_allows(h, prev, next) && prev[0] != 'n' && next[0] != 'n';
}

static void
add_segmented_candidate(candidate_list_t * out, const segment_stack_t * stack)
{
  eojeol_t        e;
  if (out->count >= HANNANUM_MAX_SEGMENT_CANDIDATES || stack->count < 2) {
    return;
  }
  e = eojeol_from_stack(stack);
  if (e.length == 0) {
    return;
  }
  if (candidate_list_has_signature(out, &e) || !candidate_list_add(out, e)) {
    free_eojeol(&e);
  }
}

static void
segment_dfs(hannanum_t * h, const char *plain, size_t offset, segment_stack_t * stack, candidate_list_t * out)
{
  size_t          len = strlen(plain);
  size_t          end;
  if (out->count >= HANNANUM_MAX_SEGMENT_CANDIDATES || stack->count >= HANNANUM_MAX_SEGMENT_DEPTH) {
    return;
  }
  if (offset == len) {
    add_segmented_candidate(out, stack);
    return;
  }
  for (end = offset + 1; end <= len; end++) {
    char           *part;
    dict_entry_t   *entry;
    size_t          i;
    int             at_start;
    int             at_end;
    if (end < len && is_utf8_continuation((unsigned char)plain[end])) {
      continue;
    }
    part = hn_strndup(plain + offset, end - offset);
    if (part == NULL) {
      return;
    }
    entry = dict_find(h->dict, part);
    if (entry == NULL) {
      free(part);
      continue;
    }
    at_start = offset == 0;
    at_end = end == len;
    for (i = 0; i < entry->candidates.count; i++) {
      const           eojeol_t *candidate = &entry->candidates.items[i];
      const char     *tag;
      if (candidate->length != 1) {
        continue;
      }
      tag = candidate->tags[0];
      if (!segment_tag_allowed(tag, at_start, at_end)) {
        continue;
      }
      if (stack->count > 0 && !segment_connection_ok(h, stack->tags[stack->count - 1], tag)) {
        continue;
      }
      if (segment_stack_push(stack, candidate->morphemes[0], tag)) {
        segment_dfs(h, plain, end, stack, out);
        segment_stack_pop(stack);
      }
    }
    free(part);
  }
}

static void HANNANUM_UNUSED
append_segmented_candidates(hannanum_t * h, const char *plain, candidate_list_t * out)
{
  segment_stack_t stack;
  memset(&stack, 0, sizeof(stack));
  segment_dfs(h, plain, 0, &stack, out);
  segment_stack_free(&stack);
}

static void
load_analyzed_dic(hannanum_t * h)
{
  char           *path = hn_path_join(h->data_dir, "kE/dic_analyzed.txt");
  FILE           *fp;
  char            line[65536];
  if (path == NULL) {
    return;
  }
  fp = fopen(path, "r");
  free(path);
  if (fp == NULL) {
    return;
  }
  while (fgets(line, sizeof(line), fp) != NULL) {
    char           *key;
    char           *analysis;
    char           *save = NULL;
    char           *alt;
    dict_entry_t   *entry;
    line[strcspn(line, "\r\n")] = '\0';
    key = trim(line);
    if (*key == '\0' || *key == '#') {
      continue;
    }
    analysis = key;
    while (*analysis != '\0' && !isspace((unsigned char)*analysis)) {
      analysis++;
    }
    if (*analysis == '\0') {
      continue;
    }
    *analysis++ = '\0';
    analysis = trim(analysis);
    entry = dict_get_or_create(h, key);
    if (entry == NULL) {
      continue;
    }
    for (alt = strtok_r(analysis, "^", &save); alt != NULL; alt = strtok_r(NULL, "^", &save)) {
      eojeol_t        e;
      if (parse_analysis(trim(alt), &e)) {
        if (!candidate_list_add(&entry->candidates, e)) {
          free_eojeol(&e);
        } else {
          entry->analyzed_count = entry->candidates.count;
        }
      }
    }
  }
  fclose(fp);
}

static void
load_surface_dic(hannanum_t * h, const char *relative, hannanum_trie_t *trie)
{
  char           *path = hn_path_join(h->data_dir, relative);
  FILE           *fp;
  char            line[65536];
  if (path == NULL) {
    return;
  }
  fp = fopen(path, "r");
  free(path);
  if (fp == NULL) {
    return;
  }
  while (fgets(line, sizeof(line), fp) != NULL) {
    char           *save = NULL;
    char           *surface;
    char           *tag;
    dict_entry_t   *entry;
    line[strcspn(line, "\r\n")] = '\0';
    surface = strtok_r(line, " \t", &save);
    if (surface == NULL || *surface == '#') {
      continue;
    }
    entry = dict_get_or_create(h, surface);
    if (entry == NULL) {
      continue;
    }
    while ((tag = strtok_r(NULL, " \t", &save)) != NULL) {
      eojeol_t        e;
      int             phoneme = -1;
      char           *dot = strchr(tag, '.');
      if (dot != NULL) {
        *dot = '\0';
        phoneme = irregular_id(h, dot + 1);
      }
      if (*tag == '\0') {
        continue;
      }
      if (trie != NULL) {
        trie_store_text(trie, surface, tag_id(h, tag), phoneme < 0 ? 0 : phoneme);
      }
      e = make_single(surface, tag);
      if (e.length > 0 && !candidate_list_add(&entry->candidates, e)) {
        free_eojeol(&e);
      }
    }
  }
  fclose(fp);
}

static void
load_probability(hannanum_t * h, prob_entry_t * *table, const char *relative)
{
  char           *path = hn_path_join(h->data_dir, relative);
  FILE           *fp;
  char            line[4096];
  if (path == NULL) {
    return;
  }
  fp = fopen(path, "r");
  free(path);
  if (fp == NULL) {
    return;
  }
  while (fgets(line, sizeof(line), fp) != NULL) {
    char           *key;
    char           *value_s;
    char           *endptr;
    double          value;
    line[strcspn(line, "\r\n")] = '\0';
    key = trim(line);
    if (*key == '\0' || *key == '#') {
      continue;
    }
    value_s = key;
    while (*value_s != '\0' && !isspace((unsigned char)*value_s)) {
      value_s++;
    }
    if (*value_s == '\0') {
      continue;
    }
    *value_s++ = '\0';
    value_s = trim(value_s);
    errno = 0;
    value = strtod(value_s, &endptr);
    if (errno == 0 && endptr != value_s) {
      prob_put(table, key, value);
    }
  }
  fclose(fp);
}
static int HANNANUM_UNUSED
is_ascii_punct_token(const char *s)
{
  if (s == NULL || s[0] == '\0' || s[1] != '\0') {
    return 0;
  }
  return strchr(".!?", s[0]) != NULL;
}

static int
is_repeated_sentence_punctuation(const char *s)
{
  size_t          i;
  if (s == NULL || *s == '\0') {
    return 0;
  }
  for (i = 0; s[i] != '\0'; i++) {
    if (s[i] != '?' && s[i] != '!') {
      return 0;
    }
  }
  return i > 1;
}

static int
is_ascii_digits(const char *s, size_t len)
{
  size_t          i;
  if (len == 0) {
    return 0;
  }
  for (i = 0; i < len; i++) {
    if (!isdigit((unsigned char)s[i])) {
      return 0;
    }
  }
  return 1;
}

static int
is_numeric_expression(const char *s)
{
  static const unsigned char automata[13][7] = {
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 9, 9, 0, 0, 2, 0 },
    { 1, 0, 0, 11, 5, 3, 0 },
    { 1, 0, 0, 11, 5, 4, 0 },
    { 1, 0, 0, 11, 5, 10, 0 },
    { 0, 0, 0, 0, 0, 6, 0 },
    { 0, 0, 0, 0, 0, 7, 0 },
    { 0, 0, 0, 0, 0, 8, 0 },
    { 1, 0, 0, 0, 5, 0, 0 },
    { 0, 0, 0, 0, 0, 10, 0 },
    { 1, 0, 0, 11, 0, 10, 0 },
    { 1, 0, 0, 0, 0, 12, 0 },
    { 1, 0, 0, 0, 0, 12, 0 }
  };
  size_t          i;
  unsigned char   state = 1;
  if (s == NULL || *s == '\0') {
    return 0;
  }
  for (i = 0; s[i] != '\0'; i++) {
    unsigned char   input = 6;
    switch (s[i]) {
    case '+':
      input = 1;
      break;
    case '-':
      input = 2;
      break;
    case '.':
      input = 3;
      break;
    case ',':
      input = 4;
      break;
    default:
      input = isdigit((unsigned char)s[i]) ? 5 : 6;
      break;
    }
    state = automata[state][input];
    if (state == 0) {
      return 0;
    }
  }
  return automata[state][0] == 1;
}

static int
is_simple_signed_digits(const char *s)
{
  size_t          i = 0;
  int             digit_count = 0;
  if (s == NULL || *s == '\0') {
    return 0;
  }
  if (s[0] == '+' || s[0] == '-') {
    i = 1;
  }
  for (; s[i] != '\0'; i++) {
    if (isdigit((unsigned char)s[i])) {
      digit_count++;
      continue;
    }
    return 0;
  }
  return digit_count > 0;
}

static int
is_ascii_abbreviation(const char *s)
{
  size_t          i;
  int             saw_period = 0;
  int             expect_letter = 1;
  if (s == NULL || *s == '\0') {
    return 0;
  }
  for (i = 0; s[i] != '\0'; i++) {
    if (expect_letter) {
      if (!isalpha((unsigned char)s[i])) {
        return 0;
      }
      expect_letter = 0;
    } else {
      if (s[i] != '.') {
        return 0;
      }
      saw_period = 1;
      expect_letter = 1;
    }
  }
  return saw_period && expect_letter;
}

static int
is_ascii_alpha_token(const char *s)
{
  size_t          i;
  if (s == NULL || *s == '\0') {
    return 0;
  }
  for (i = 0; s[i] != '\0'; i++) {
    if (!isalpha((unsigned char)s[i])) {
      return 0;
    }
  }
  return 1;
}

static int
append_ascii_abbreviation_candidate(const char *plain, candidate_list_t * list)
{
  size_t          len = strlen(plain);
  size_t          count = 0;
  size_t          i;
  eojeol_t        e;
  if (!is_ascii_abbreviation(plain)) {
    return 0;
  }
  for (i = 0; i < len; i++) {
    if (isalpha((unsigned char)plain[i])) {
      count += 2;
    }
  }
  e.morphemes = (char **)calloc(count, sizeof(char *));
  e.tags = (char **)calloc(count, sizeof(char *));
  if (e.morphemes == NULL || e.tags == NULL) {
    free(e.morphemes);
    free(e.tags);
    return 0;
  }
  e.length = count;
  count = 0;
  for (i = 0; i < len; i++) {
    char            letter[2];
    if (!isalpha((unsigned char)plain[i])) {
      continue;
    }
    letter[0] = plain[i];
    letter[1] = '\0';
    e.morphemes[count] = hn_strdup(letter);
    e.tags[count++] = hn_strdup("f");
    e.morphemes[count] = hn_strdup(".");
    e.tags[count++] = hn_strdup("sf");
  }
  for (i = 0; i < e.length; i++) {
    if (e.morphemes[i] == NULL || e.tags[i] == NULL) {
      free_eojeol(&e);
      return 0;
    }
  }
  if (!candidate_list_add(list, e)) {
    free_eojeol(&e);
    return 0;
  }
  return 1;
}

static int
append_repeated_punctuation_candidate(const char *plain, candidate_list_t * list)
{
  size_t          len = strlen(plain);
  size_t          i;
  eojeol_t        e;
  if (!is_repeated_sentence_punctuation(plain)) {
    return 0;
  }
  e.morphemes = (char **)calloc(len, sizeof(char *));
  e.tags = (char **)calloc(len, sizeof(char *));
  if (e.morphemes == NULL || e.tags == NULL) {
    free(e.morphemes);
    free(e.tags);
    return 0;
  }
  e.length = len;
  for (i = 0; i < len; i++) {
    char            mark[2];
    mark[0] = plain[i];
    mark[1] = '\0';
    e.morphemes[i] = hn_strdup(mark);
    e.tags[i] = hn_strdup("sf");
    if (e.morphemes[i] == NULL || e.tags[i] == NULL) {
      free_eojeol(&e);
      return 0;
    }
  }
  if (!candidate_list_add(list, e)) {
    free_eojeol(&e);
    return 0;
  }
  return 1;
}

static int
append_email_head_candidate(const char *plain, candidate_list_t * list)
{
  const char     *at = strchr(plain, '@');
  const char     *dot = strrchr(plain, '.');
  const char     *surfaces[4];
  const char     *tags[4] = { "f", "sy", "f", "sy" };
  char           *local;
  char           *domain;
  eojeol_t        e;
  if (at == NULL || dot == NULL || dot < at || dot[1] != '\0' || at == plain || at + 1 == dot) {
    return 0;
  }
  local = hn_strndup(plain, (size_t) (at - plain));
  domain = hn_strndup(at + 1, (size_t) (dot - at - 1));
  if (local == NULL || domain == NULL) {
    free(local);
    free(domain);
    return 0;
  }
  surfaces[0] = local;
  surfaces[1] = "@";
  surfaces[2] = domain;
  surfaces[3] = ".";
  e = make_parts(4, surfaces, tags);
  free(local);
  free(domain);
  if (e.length > 0) {
    candidate_list_add(list, e);
    return 1;
  }
  return 0;
}

static int
is_number_unit(const char *s)
{
  return strcmp(s, "년") == 0 || strcmp(s, "월") == 0 || strcmp(s, "일") == 0 || strcmp(s, "시") == 0;
}

static int
korean_number_syllable_tag(const char *syllable, int has_previous, const char **tag)
{
  if (strcmp(syllable, "일") == 0 && has_previous) {
    *tag = "nbu";
    return 1;
  }
  if (strcmp(syllable, "영") == 0 || strcmp(syllable, "공") == 0 || strcmp(syllable, "일") == 0 || strcmp(syllable, "이") == 0 || strcmp(syllable, "삼") == 0 || strcmp(syllable, "사") == 0 || strcmp(syllable, "오") == 0 || strcmp(syllable, "육") == 0 || strcmp(syllable, "륙") == 0 || strcmp(syllable, "칠") == 0 || strcmp(syllable, "팔") == 0 || strcmp(syllable, "구") == 0 || strcmp(syllable, "십") == 0 || strcmp(syllable, "백") == 0 || strcmp(syllable, "천") == 0 || strcmp(syllable, "만") == 0 || strcmp(syllable, "억") == 0 || strcmp(syllable, "조") == 0) {
    *tag = "nnc";
    return 1;
  }
  return 0;
}

static int
append_korean_number_candidate(const char *plain, candidate_list_t *list)
{
  const unsigned char *p = (const unsigned char *)plain;
  eojeol_t e;
  size_t capacity = 0;
  size_t count = 0;
  memset(&e, 0, sizeof(e));
  while (*p != '\0') {
    unsigned int cp;
    size_t width;
    char *syllable;
    const char *tag;
    if (!utf8_decode_one(p, &cp, &width) || cp < 0xac00 || cp > 0xd7a3) {
      free_eojeol(&e);
      return 0;
    }
    syllable = hn_strndup((const char *)p, width);
    if (syllable == NULL) {
      free_eojeol(&e);
      return 0;
    }
    if (!korean_number_syllable_tag(syllable, count > 0, &tag)) {
      free(syllable);
      free_eojeol(&e);
      return 0;
    }
    if (count == capacity) {
      char **next_m;
      char **next_t;
      size_t new_capacity = capacity == 0 ? 4 : capacity * 2;
      next_m = (char **)realloc(e.morphemes, new_capacity * sizeof(char *));
      if (next_m == NULL) {
        free(syllable);
        free_eojeol(&e);
        return 0;
      }
      e.morphemes = next_m;
      next_t = (char **)realloc(e.tags, new_capacity * sizeof(char *));
      if (next_t == NULL) {
        free(syllable);
        free_eojeol(&e);
        return 0;
      }
      e.tags = next_t;
      capacity = new_capacity;
    }
    e.morphemes[count] = syllable;
    e.tags[count] = hn_strdup(tag);
    if (e.tags[count] == NULL) {
      free_eojeol(&e);
      return 0;
    }
    count++;
    e.length = count;
    p += width;
  }
  if (count < 2) {
    free_eojeol(&e);
    return 0;
  }
  if (!candidate_list_add(list, e)) {
    free_eojeol(&e);
    return 0;
  }
  return 1;
}

static int
entry_has_noun_tag(const dict_entry_t *entry)
{
  size_t i;
  if (entry == NULL) {
    return 0;
  }
  for (i = 0; i < entry->candidates.count; i++) {
    const eojeol_t *candidate = &entry->candidates.items[i];
    if (candidate->length == 1 && candidate->tags[0][0] == 'n') {
      return 1;
    }
  }
  return 0;
}

static const char *
entry_preferred_noun_tag(const dict_entry_t *entry)
{
  size_t i;
  if (entry == NULL) {
    return NULL;
  }
  for (i = 0; i < entry->candidates.count; i++) {
    const eojeol_t *candidate = &entry->candidates.items[i];
    if (candidate->length == 1 && starts_with(candidate->tags[0], "ncn")) {
      return candidate->tags[0];
    }
  }
  for (i = 0; i < entry->candidates.count; i++) {
    const eojeol_t *candidate = &entry->candidates.items[i];
    if (candidate->length == 1 && candidate->tags[0][0] == 'n') {
      return candidate->tags[0];
    }
  }
  return NULL;
}

static int
append_parenthesized_prefix_candidate(hannanum_t *h, const char *plain, candidate_list_t *list)
{
  const char *close = strchr(plain, ')');
  char *prefix;
  const char *rest;
  dict_entry_t *prefix_entry;
  dict_entry_t *rest_entry;
  const char *rest_tag;
  eojeol_t e;
  if (plain[0] != '(' || close == NULL || close[1] == '\0') {
    return 0;
  }
  prefix = hn_strndup(plain, (size_t)(close - plain + 1));
  if (prefix == NULL) {
    return 0;
  }
  rest = close + 1;
  prefix_entry = dict_find(h->dict, prefix);
  rest_entry = dict_find(h->dict, rest);
  rest_tag = entry_preferred_noun_tag(rest_entry);
  if (prefix_entry == NULL || !entry_has_noun_tag(prefix_entry) || rest_tag == NULL) {
    free(prefix);
    return 0;
  }
  e = make_pair(prefix, "sr", rest, rest_tag);
  free(prefix);
  if (e.length == 0) {
    return 0;
  }
  if (!candidate_list_add(list, e)) {
    free_eojeol(&e);
    return 0;
  }
  return 1;
}

static int
entry_has_yong_tag(const dict_entry_t *entry)
{
  size_t i;
  if (entry == NULL) {
    return 0;
  }
  for (i = 0; i < entry->candidates.count; i++) {
    const eojeol_t *candidate = &entry->candidates.items[i];
    if (candidate->length == 1 && (candidate->tags[0][0] == 'p' || starts_with(candidate->tags[0], "xsm") || starts_with(candidate->tags[0], "xsv"))) {
      return 1;
    }
  }
  return 0;
}

static const char *
entry_preferred_yong_tag(const dict_entry_t *entry)
{
  size_t i;
  if (entry == NULL) {
    return NULL;
  }
  for (i = 0; i < entry->candidates.count; i++) {
    const eojeol_t *candidate = &entry->candidates.items[i];
    if (candidate->length == 1 && starts_with(candidate->tags[0], "px")) {
      return candidate->tags[0];
    }
  }
  for (i = 0; i < entry->candidates.count; i++) {
    const eojeol_t *candidate = &entry->candidates.items[i];
    if (candidate->length == 1 && starts_with(candidate->tags[0], "pvg")) {
      return candidate->tags[0];
    }
  }
  for (i = 0; i < entry->candidates.count; i++) {
    const eojeol_t *candidate = &entry->candidates.items[i];
    if (candidate->length == 1 && (candidate->tags[0][0] == 'p' || starts_with(candidate->tags[0], "xsm") || starts_with(candidate->tags[0], "xsv"))) {
      return candidate->tags[0];
    }
  }
  return NULL;
}

static int
append_stem_ending_candidate(hannanum_t *h, candidate_list_t *list, const char *stem, const char *ending, const char *ending_tag)
{
  dict_entry_t *entry = dict_find(h->dict, stem);
  const char *stem_tag = entry_preferred_yong_tag(entry);
  eojeol_t e;
  if (stem_tag == NULL) {
    return 0;
  }
  e = make_pair(stem, stem_tag, ending, ending_tag);
  if (e.length == 0) {
    return 0;
  }
  if (!candidate_list_has_signature(list, &e)) {
    if (!candidate_list_add(list, e)) {
      free_eojeol(&e);
      return 0;
    }
  } else {
    free_eojeol(&e);
  }
  return 1;
}

static char *
stem_without_final_jong(const char *plain, size_t syllable_offset, size_t syllable_width)
{
  unsigned int cp;
  size_t width;
  unsigned int index;
  unsigned int without_final;
  char *stem = NULL;
  size_t used = 0;
  size_t capacity = 0;
  if (!utf8_decode_one((const unsigned char *)plain + syllable_offset, &cp, &width) || width != syllable_width || cp < 0xac00 || cp > 0xd7a3) {
    return NULL;
  }
  index = cp - 0xac00;
  if (index % 28 == 0) {
    return NULL;
  }
  without_final = cp - (index % 28);
  if (!hn_str_append(&stem, &used, &capacity, plain, syllable_offset)) {
    free(stem);
    return NULL;
  }
  if (!hn_str_append_utf8(&stem, &used, &capacity, without_final)) {
    free(stem);
    return NULL;
  }
  return stem;
}

static int HANNANUM_UNUSED
append_conjugation_candidates(hannanum_t *h, const char *plain, candidate_list_t *list)
{
  size_t len = strlen(plain);
  int added = 0;
  if (len > 3 && strcmp(plain + len - strlen("고"), "고") == 0) {
    char *stem = hn_strndup(plain, len - strlen("고"));
    if (stem != NULL) {
      if (append_stem_ending_candidate(h, list, stem, "고", "ecs")) {
        added = 1;
      }
      free(stem);
    }
  }
  if (len > 3 && strcmp(plain + len - strlen("라"), "라") == 0) {
    char *stem = hn_strndup(plain, len - strlen("라"));
    if (stem != NULL) {
      if (entry_has_yong_tag(dict_find(h->dict, stem)) && hangul_has_positive_vowel(stem)) {
        if (append_stem_ending_candidate(h, list, stem, "아", "ecs")) {
          added = 1;
        }
      }
      free(stem);
    }
  }
  if (len > 3 && strcmp(plain + len - strlen("다"), "다") == 0) {
    size_t end = len - strlen("다");
    size_t start = 0;
    size_t width = 0;
    while (start < end) {
      unsigned int cp;
      size_t w;
      if (!utf8_decode_one((const unsigned char *)plain + start, &cp, &w) || start + w > end) {
        break;
      }
      width = w;
      if (start + w == end) {
        char *stem = stem_without_final_jong(plain, start, width);
        if (stem != NULL) {
          if (append_stem_ending_candidate(h, list, stem, "ㄴ다", "ef")) {
            added = 1;
          }
          free(stem);
        }
        break;
      }
      start += w;
    }
  }
  return added;
}

static int HANNANUM_UNUSED
append_special_candidate(hannanum_t *h, const char *plain, candidate_list_t * list)
{
  size_t          len = strlen(plain);
  size_t          i;
  eojeol_t        e;
  if (len > 1 && plain[len - 1] == ':') {
    char           *base = hn_strndup(plain, len - 1);
    if (base == NULL) {
      return 0;
    }
    e = make_pair(base, "ncn", ":", "sp");
    free(base);
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (append_parenthesized_prefix_candidate(h, plain, list)) {
    return 1;
  }
  if (strcmp(plain, "...") == 0) {
    e = make_single(plain, "se");
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (strcmp(plain, ".....") == 0) {
    const char     *surfaces[3] = { ".", "...", "." };
    const char     *tags[3] = { "sy", "se", "sy" };
    e = make_parts(3, surfaces, tags);
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (strcmp(plain, ".\"") == 0) {
    const char     *surfaces[2] = { ".", "\"" };
    const char     *tags[2] = { "sf", "sl" };
    e = make_parts(2, surfaces, tags);
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (strcmp(plain, "..") == 0) {
    const char     *surfaces[2] = { ".", "." };
    const char     *tags[2] = { "sf", "sf" };
    e = make_parts(2, surfaces, tags);
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (strcmp(plain, "(") == 0) {
    e = make_single(plain, "sl");
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (strcmp(plain, ")") == 0) {
    e = make_single(plain, "sr");
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (append_repeated_punctuation_candidate(plain, list)) {
    return 1;
  }
  if (append_email_head_candidate(plain, list)) {
    return 1;
  }
  if (len >= 2 && plain[0] == '"' && plain[len - 1] == '"') {
    char           *inner = hn_strndup(plain + 1, len - 2);
    const char     *surfaces[3];
    const char     *tags[3] = { "sl", "ncn", "sr" };
    if (inner == NULL) {
      return 0;
    }
    surfaces[0] = "\"";
    surfaces[1] = inner;
    surfaces[2] = "\"";
    e = make_parts(3, surfaces, tags);
    free(inner);
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (len >= 2 && plain[0] == '\'' && plain[len - 1] == '\'') {
    char           *inner = hn_strndup(plain + 1, len - 2);
    const char     *surfaces[3];
    const char     *tags[3] = { "sl", "ncn", "sr" };
    if (inner == NULL) {
      return 0;
    }
    surfaces[0] = "'";
    surfaces[1] = inner;
    surfaces[2] = "'";
    e = make_parts(3, surfaces, tags);
    free(inner);
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (len > 1 && plain[len - 1] == '%' && is_ascii_digits(plain, len - 1)) {
    char           *number = hn_strndup(plain, len - 1);
    if (number == NULL) {
      return 0;
    }
    e = make_pair(number, "nnc", "%", "su");
    free(number);
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (len > 1 && plain[len - 1] == '.' && strchr(plain, ',') != NULL) {
    char           *number = hn_strndup(plain, len - 1);
    if (number == NULL) {
      return 0;
    }
    if (is_numeric_expression(number) || is_simple_signed_digits(number)) {
      e = make_pair(number, "nnc", ".", "sf");
      free(number);
      if (e.length > 0) {
        candidate_list_add(list, e);
        return 1;
      }
    } else {
      free(number);
    }
  }
  if (is_numeric_expression(plain) || is_simple_signed_digits(plain)) {
    e = make_single(plain, "nnc");
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  if (append_korean_number_candidate(plain, list)) {
    return 1;
  }
  if (len > 1 && plain[len - 1] == '.') {
    char           *number = hn_strndup(plain, len);
    if (number == NULL) {
      return 0;
    }
    if (is_numeric_expression(number)) {
      e = make_single(number, "nnc");
      free(number);
      if (e.length > 0) {
        candidate_list_add(list, e);
        return 1;
      }
    } else {
      free(number);
    }
  }
  if (append_ascii_abbreviation_candidate(plain, list)) {
    return 1;
  }
  if (is_ascii_alpha_token(plain)) {
    e = make_single(plain, "f");
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  for (i = 0; i < len; i++) {
    if (!isdigit((unsigned char)plain[i])) {
      break;
    }
  }
  if (i > 0 && i < len && is_ascii_digits(plain, i) && is_number_unit(plain + i)) {
    char           *number = hn_strndup(plain, i);
    if (number == NULL) {
      return 0;
    }
    e = make_pair(number, "nnc", plain + i, "nbu");
    free(number);
    if (e.length > 0) {
      candidate_list_add(list, e);
      return 1;
    }
  }
  return 0;
}

static int
chart_tag_type_check(hannanum_t *h, int tag_type, int tag, void *userdata)
{
  (void)userdata;
  return check_tag_type(h, tag_type, tag);
}

static int
chart_connection_check(hannanum_t *h, const morpheme_chart_node_t *left, const morpheme_chart_node_t *right, void *userdata)
{
  const char *left_name;
  const char *right_name;
  codepoint_vec_t left_triple;
  codepoint_vec_t right_triple;
  int allowed;
  (void)userdata;
  if (left->tag < 0 || right->tag < 0 || (size_t)left->tag >= h->tag_count || (size_t)right->tag >= h->tag_count) {
    return 0;
  }
  left_name = h->tag_names[left->tag];
  right_name = h->tag_names[right->tag];
  if ((starts_with(left_name, "nc") || left_name[0] == 'f') && right_name[0] == 'n') {
    memset(&left_triple, 0, sizeof(left_triple));
    memset(&right_triple, 0, sizeof(right_triple));
    if (starts_with(right_name, "nq")) {
      return 0;
    }
    if (!hannanum_code_to_triple(left->str, &left_triple) || !hannanum_code_to_triple(right->str, &right_triple)) {
      codepoint_vec_free(&left_triple);
      codepoint_vec_free(&right_triple);
      return 0;
    }
    allowed = left_triple.count >= 4 && right_triple.count >= 2;
    codepoint_vec_free(&left_triple);
    codepoint_vec_free(&right_triple);
    if (!allowed) {
      return 0;
    }
  }
  return connection_allows(h, left_name, right_name);
}

static int
chart_remaining_from_position(segment_position_t *sp, int from, codepoint_vec_t *remaining)
{
  int pos;
  memset(remaining, 0, sizeof(*remaining));
  if (sp == NULL || from < 0) {
    return 0;
  }
  for (pos = from; pos != (int)HANNANUM_POSITION_START_KEY; pos = segment_position_next(sp, pos)) {
    segment_position_node_t *node = segment_position_get(sp, pos);
    if (node == NULL) {
      codepoint_vec_free(remaining);
      return 0;
    }
    if (!codepoint_vec_push(remaining, node->key)) {
      codepoint_vec_free(remaining);
      return 0;
    }
  }
  return 1;
}

static int
chart_add_parenthesized_prefix(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp)
{
  int sr = tag_id(h, "sr");
  int pos;
  codepoint_vec_t prefix;
  int added = 0;
  segment_position_node_t *from_pos = segment_position_get(sp, 1);
  if (sr < 0 || from_pos == NULL) {
    return 0;
  }
  memset(&prefix, 0, sizeof(prefix));
  for (pos = 1; pos != (int)HANNANUM_POSITION_START_KEY; pos = segment_position_next(sp, pos)) {
    segment_position_node_t *node = segment_position_get(sp, pos);
    char *text;
    int next;
    int chart_node;
    if (node == NULL) {
      break;
    }
    if (!codepoint_vec_push(&prefix, node->key)) {
      break;
    }
    text = hannanum_code_from_triple(&prefix);
    if (text == NULL) {
      break;
    }
    next = segment_position_next(sp, pos);
    if (text[0] == '(' && strchr(text, ')') != NULL && next != 0 && dict_find(h->dict, text) != NULL && !morpheme_chart_check(chart, from_pos->morpheme, from_pos->morph_count, sr, 0, next, 0, text) && from_pos->morph_count < HANNANUM_MAX_MORPHEME_COUNT) {
      chart_node = morpheme_chart_add(chart, sr, 0, next, 0, text);
      if (chart_node >= 0) {
        from_pos->morpheme[from_pos->morph_count++] = chart_node;
        added++;
      }
    }
    free(text);
  }
  codepoint_vec_free(&prefix);
  if (added > 0 && from_pos->state == HANNANUM_SP_STATE_N) {
    from_pos->state = HANNANUM_SP_STATE_D;
  }
  return added;
}

static int
chart_expander(hannanum_t *h, morpheme_chart_t *chart, segment_position_t *sp, simti_t *simti, int from, const char *morph_text, void *userdata)
{
  exp_irregular_ids_t ids;
  codepoint_vec_t prev;
  codepoint_vec_t remaining;
  exp_change_t changes[64];
  size_t count;
  int added;
  (void)userdata;
  memset(&ids, 0, sizeof(ids));
  ids.type_b = irregular_id(h, "irrb");
  ids.type_s = irregular_id(h, "irrs");
  ids.type_d = irregular_id(h, "irrd");
  ids.type_h = irregular_id(h, "irrh");
  ids.type_reu = irregular_id(h, "irrlu");
  ids.type_reo = irregular_id(h, "irrle");
  memset(&prev, 0, sizeof(prev));
  memset(&remaining, 0, sizeof(remaining));
  if (!hannanum_code_to_triple(morph_text, &prev)) {
    return 0;
  }
  if (!chart_remaining_from_position(sp, from, &remaining)) {
    codepoint_vec_free(&prev);
    return 0;
  }
  count = exp_prule_generate(&prev, &remaining, &ids, changes, 64);
  added = exp_apply_changes_to_chart(h, chart, sp, simti, from, changes, count);
  while (count > 0) {
    exp_change_free(&changes[--count]);
  }
  codepoint_vec_free(&prev);
  codepoint_vec_free(&remaining);
  return added;
}

static candidate_list_t HANNANUM_UNUSED
chart_candidates_for(hannanum_t *h, const char *plain)
{
  candidate_list_t list;
  morpheme_chart_t chart;
  segment_position_t sp;
  simti_t *simti;
  int iwg = tag_id(h, "iwg");
  memset(&list, 0, sizeof(list));
  if (iwg < 0 || !morpheme_chart_init(&chart)) {
    return list;
  }
  simti = simti_create();
  if (simti == NULL) {
    return list;
  }
  if (morpheme_chart_init_word(&chart, &sp, simti, plain, iwg)) {
    int pass;
    chart_add_parenthesized_prefix(h, &chart, &sp);
    int result_count = 0;
    for (pass = 0; pass < 8; pass++) {
      int current = morpheme_chart_analyze_with_callbacks(h, &chart, &sp, simti, 0, HANNANUM_TAG_TYPE_ALL, chart_expander, chart_tag_type_check, chart_connection_check, NULL);
      if (current == result_count) {
        break;
      }
      result_count = current;
    }
    if (result_count == 0) {
      int unk = tag_id(h, "unk");
      if (morpheme_chart_analyze_unknown(h, &chart, &sp, unk) > 0) {
        for (pass = 0; pass < 8; pass++) {
          int current = morpheme_chart_analyze_with_callbacks(h, &chart, &sp, simti, 0, HANNANUM_TAG_TYPE_ALL, chart_expander, chart_tag_type_check, chart_connection_check, NULL);
          if (current == result_count) {
            break;
          }
          result_count = current;
        }
      }
    }
    morpheme_chart_collect_results(&chart, 0, h->tag_names, h->tag_count, &list);
  }
  simti_destroy(simti);
  morpheme_chart_clear(&chart);
  return list;
}
