static int
tag_id(hannanum_t *h, const char *tag)
{
  size_t i;
  if (h == NULL || tag == NULL) {
    return -1;
  }
  for (i = 0; i < h->tag_count; i++) {
    if (strcmp(h->tag_names[i], tag) == 0) {
      return (int)i;
    }
  }
  return -1;
}

static tag_group_t *
tag_group_find(hannanum_t *h, const char *name)
{
  size_t i;
  if (h == NULL || name == NULL) {
    return NULL;
  }
  for (i = 0; i < h->tag_group_count; i++) {
    if (strcmp(h->tag_groups[i].name, name) == 0) {
      return &h->tag_groups[i];
    }
  }
  return NULL;
}

static int HANNANUM_UNUSED
irregular_id(hannanum_t *h, const char *name)
{
  size_t i;
  if (h == NULL || name == NULL) {
    return -1;
  }
  for (i = 0; i < h->irregular_count; i++) {
    if (strcmp(h->irregular_names[i], name) == 0) {
      return (int)i;
    }
  }
  return -1;
}

static int
int_list_contains(const int *items, size_t count, int value)
{
  size_t i;
  for (i = 0; i < count; i++) {
    if (items[i] == value) {
      return 1;
    }
  }
  return 0;
}

static int
int_list_add_unique(int **items, size_t *count, size_t *capacity, int value)
{
  int *next;
  if (value < 0 || int_list_contains(*items, *count, value)) {
    return 1;
  }
  if (*count == *capacity) {
    size_t new_capacity = *capacity == 0 ? 8 : *capacity * 2;
    next = (int *)realloc(*items, new_capacity * sizeof(int));
    if (next == NULL) {
      return 0;
    }
    *items = next;
    *capacity = new_capacity;
  }
  (*items)[(*count)++] = value;
  return 1;
}

static void HANNANUM_UNUSED
int_list_remove(int *items, size_t *count, int value)
{
  size_t i;
  for (i = 0; i < *count; i++) {
    if (items[i] == value) {
      memmove(items + i, items + i + 1, (*count - i - 1) * sizeof(int));
      (*count)--;
      return;
    }
  }
}

static int
append_tag_or_group(hannanum_t *h, const char *name, int **items, size_t *count, size_t *capacity)
{
  tag_group_t *group = tag_group_find(h, name);
  size_t i;
  if (group != NULL) {
    for (i = 0; i < group->count; i++) {
      if (!int_list_add_unique(items, count, capacity, group->ids[i])) {
        return 0;
      }
    }
    return 1;
  }
  return int_list_add_unique(items, count, capacity, tag_id(h, name));
}

static int
tag_type_add(hannanum_t *h, int type, int tag)
{
  size_t capacity = h->tag_types[type].count;
  return int_list_add_unique(&h->tag_types[type].ids, &h->tag_types[type].count, &capacity, tag);
}

static int
tag_type_add_group(hannanum_t *h, int type, const char *group_name)
{
  tag_group_t *group = tag_group_find(h, group_name);
  size_t i;
  if (group == NULL) {
    return 0;
  }
  for (i = 0; i < group->count; i++) {
    if (!tag_type_add(h, type, group->ids[i])) {
      return 0;
    }
  }
  return 1;
}

static int
set_tag_types_kaist(hannanum_t *h)
{
  tag_type_add_group(h, HANNANUM_TAG_TYPE_VERBS, "pv");
  tag_type_add_group(h, HANNANUM_TAG_TYPE_VERBS, "xsm");
  tag_type_add(h, HANNANUM_TAG_TYPE_VERBS, tag_id(h, "px"));
  tag_type_add_group(h, HANNANUM_TAG_TYPE_NOUNS, "n");
  tag_type_add_group(h, HANNANUM_TAG_TYPE_NPS, "np");
  tag_type_add_group(h, HANNANUM_TAG_TYPE_ADJS, "pa");
  tag_type_add_group(h, HANNANUM_TAG_TYPE_EOMIES, "e");
  tag_type_add_group(h, HANNANUM_TAG_TYPE_YONGS, "p");
  tag_type_add_group(h, HANNANUM_TAG_TYPE_YONGS, "xsv");
  tag_type_add_group(h, HANNANUM_TAG_TYPE_YONGS, "xsm");
  tag_type_add(h, HANNANUM_TAG_TYPE_YONGS, tag_id(h, "ep"));
  tag_type_add(h, HANNANUM_TAG_TYPE_YONGS, tag_id(h, "jp"));
  tag_type_add(h, HANNANUM_TAG_TYPE_JP, tag_id(h, "jp"));
  tag_type_add(h, HANNANUM_TAG_TYPE_NBNP, tag_id(h, "nbn"));
  tag_type_add(h, HANNANUM_TAG_TYPE_NBNP, tag_id(h, "npd"));
  tag_type_add(h, HANNANUM_TAG_TYPE_NBNP, tag_id(h, "npp"));
  tag_type_add(h, HANNANUM_TAG_TYPE_JOSA, tag_id(h, "jxc"));
  tag_type_add(h, HANNANUM_TAG_TYPE_JOSA, tag_id(h, "jco"));
  tag_type_add(h, HANNANUM_TAG_TYPE_JOSA, tag_id(h, "jca"));
  tag_type_add(h, HANNANUM_TAG_TYPE_JOSA, tag_id(h, "jcm"));
  tag_type_add(h, HANNANUM_TAG_TYPE_JOSA, tag_id(h, "jcs"));
  tag_type_add(h, HANNANUM_TAG_TYPE_JOSA, tag_id(h, "jcc"));
  return 1;
}

static int HANNANUM_UNUSED
check_tag_type(hannanum_t *h, int type, int tag)
{
  if (type == HANNANUM_TAG_TYPE_ALL) {
    return 1;
  }
  if (type < 0 || type >= HANNANUM_TAG_TYPE_COUNT) {
    return 0;
  }
  return int_list_contains(h->tag_types[type].ids, h->tag_types[type].count, tag);
}

static int HANNANUM_UNUSED
check_phoneme_type(int phoneme_type, int phoneme)
{
  if (phoneme_type == 0) {
    return 1;
  }
  return phoneme_type == phoneme;
}

static int
load_tag_set(hannanum_t *h)
{
  char *path = hn_path_join(h->data_dir, "kE/tag_set.txt");
  FILE *fp;
  char line[4096];
  if (path == NULL) {
    return 0;
  }
  fp = fopen(path, "r");
  free(path);
  if (fp == NULL) {
    return 0;
  }
  while (fgets(line, sizeof(line), fp) != NULL) {
    char *kind;
    char *name;
    char *rest;
    char *comment;
    line[strcspn(line, "\r\n")] = '\0';
    comment = strstr(line, "//");
    if (comment != NULL) {
      *comment = '\0';
    }
    kind = strtok(line, " \t");
    if (kind == NULL) {
      continue;
    }
    if (strcmp(kind, "TAG") == 0) {
      char **next;
      name = strtok(NULL, " \t");
      if (name == NULL) {
        continue;
      }
      next = (char **)realloc(h->tag_names, (h->tag_count + 1) * sizeof(char *));
      if (next == NULL) {
        fclose(fp);
        return 0;
      }
      h->tag_names = next;
      h->tag_names[h->tag_count] = hn_strdup(name);
      if (h->tag_names[h->tag_count] == NULL) {
        fclose(fp);
        return 0;
      }
      h->tag_count++;
    } else if (strcmp(kind, "TSET") == 0) {
      tag_group_t *next_groups;
      int *ids = NULL;
      size_t count = 0;
      size_t capacity = 0;
      char *token;
      name = strtok(NULL, " \t");
      rest = strtok(NULL, "");
      if (name == NULL || rest == NULL) {
        continue;
      }
      token = strtok(rest, " \t");
      while (token != NULL) {
        if (!append_tag_or_group(h, token, &ids, &count, &capacity)) {
          free(ids);
          fclose(fp);
          return 0;
        }
        token = strtok(NULL, " \t");
      }
      next_groups = (tag_group_t *)realloc(h->tag_groups, (h->tag_group_count + 1) * sizeof(tag_group_t));
      if (next_groups == NULL) {
        free(ids);
        fclose(fp);
        return 0;
      }
      h->tag_groups = next_groups;
      h->tag_groups[h->tag_group_count].name = hn_strdup(name);
      h->tag_groups[h->tag_group_count].ids = ids;
      h->tag_groups[h->tag_group_count].count = count;
      if (h->tag_groups[h->tag_group_count].name == NULL) {
        fclose(fp);
        return 0;
      }
      h->tag_group_count++;
    } else if (strcmp(kind, "IRR") == 0) {
      char **next;
      name = strtok(NULL, " \t");
      if (name == NULL) {
        continue;
      }
      next = (char **)realloc(h->irregular_names, (h->irregular_count + 1) * sizeof(char *));
      if (next == NULL) {
        fclose(fp);
        return 0;
      }
      h->irregular_names = next;
      h->irregular_names[h->irregular_count] = hn_strdup(name);
      if (h->irregular_names[h->irregular_count] == NULL) {
        fclose(fp);
        return 0;
      }
      h->irregular_count++;
    }
  }
  fclose(fp);
  set_tag_types_kaist(h);
  return h->tag_count > 0;
}

static void
free_tag_set(hannanum_t *h)
{
  size_t i;
  if (h == NULL) {
    return;
  }
  for (i = 0; i < h->tag_count; i++) {
    free(h->tag_names[i]);
  }
  free(h->tag_names);
  h->tag_names = NULL;
  h->tag_count = 0;
  for (i = 0; i < h->irregular_count; i++) {
    free(h->irregular_names[i]);
  }
  free(h->irregular_names);
  h->irregular_names = NULL;
  h->irregular_count = 0;
  for (i = 0; i < h->tag_group_count; i++) {
    free(h->tag_groups[i].name);
    free(h->tag_groups[i].ids);
  }
  free(h->tag_groups);
  h->tag_groups = NULL;
  h->tag_group_count = 0;
  for (i = 0; i < HANNANUM_TAG_TYPE_COUNT; i++) {
    free(h->tag_types[i].ids);
    h->tag_types[i].ids = NULL;
    h->tag_types[i].count = 0;
  }
}
