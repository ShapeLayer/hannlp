static int
parse_connection_side(hannanum_t *h, const char *expr, int **items, size_t *count)
{
  char *copy = hn_strdup(expr);
  char *token;
  char *save = NULL;
  size_t capacity = h->tag_count;
  if (copy == NULL) {
    return 0;
  }
  if (*items == NULL) {
    *items = (int *)calloc(capacity == 0 ? 1 : capacity, sizeof(int));
    if (*items == NULL) {
      free(copy);
      return 0;
    }
  }
  for (token = strtok_r(copy, ",()", &save); token != NULL; token = strtok_r(NULL, ",()", &save)) {
    char *minus;
    char *base = token;
    int *before = NULL;
    size_t before_count;
    minus = strchr(token, '-');
    if (minus != NULL) {
      *minus = '\0';
    }
    before_count = *count;
    if (!append_tag_or_group(h, base, items, count, &capacity)) {
      free(copy);
      return 0;
    }
    if (minus != NULL) {
      char *remove_token;
      char *remove_save = NULL;
      before = (int *)calloc(*count - before_count, sizeof(int));
      if (before == NULL && *count > before_count) {
        free(copy);
        return 0;
      }
      memcpy(before, *items + before_count, (*count - before_count) * sizeof(int));
      for (remove_token = strtok_r(minus + 1, "-", &remove_save); remove_token != NULL; remove_token = strtok_r(NULL, "-", &remove_save)) {
        tag_group_t *group = tag_group_find(h, remove_token);
        if (group != NULL) {
          size_t i;
          for (i = 0; i < group->count; i++) {
            int_list_remove(*items, count, group->ids[i]);
          }
        } else {
          int_list_remove(*items, count, tag_id(h, remove_token));
        }
      }
      free(before);
    }
  }
  free(copy);
  return 1;
}

static int
load_connections(hannanum_t *h)
{
  char *path;
  FILE *fp;
  char line[4096];
  if (h->tag_count == 0) {
    return 0;
  }
  h->connections = (unsigned char *)calloc(h->tag_count * h->tag_count, sizeof(unsigned char));
  if (h->connections == NULL) {
    return 0;
  }
  path = hn_path_join(h->data_dir, "kE/connections.txt");
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
    char *expr;
    char *star;
    int *left = NULL;
    int *right = NULL;
    size_t left_count = 0;
    size_t right_count = 0;
    size_t i;
    size_t j;
    line[strcspn(line, "\r\n")] = '\0';
    kind = strtok(line, " \t");
    if (kind == NULL || strcmp(kind, "CONNECTION") != 0) {
      continue;
    }
    expr = strtok(NULL, " \t");
    if (expr == NULL) {
      continue;
    }
    star = strchr(expr, '*');
    if (star == NULL) {
      continue;
    }
    *star = '\0';
    if (!parse_connection_side(h, expr, &left, &left_count) || !parse_connection_side(h, star + 1, &right, &right_count)) {
      free(left);
      free(right);
      fclose(fp);
      return 0;
    }
    for (i = 0; i < left_count; i++) {
      for (j = 0; j < right_count; j++) {
        if (left[i] >= 0 && right[j] >= 0) {
          h->connections[(size_t)left[i] * h->tag_count + (size_t)right[j]] = 1;
        }
      }
    }
    free(left);
    free(right);
  }
  fclose(fp);
  return 1;
}

static int
connection_allows(hannanum_t *h, const char *left_tag, const char *right_tag)
{
  int left;
  int right;
  if (h == NULL || h->connections == NULL) {
    return 0;
  }
  left = tag_id(h, left_tag);
  right = tag_id(h, right_tag);
  if (left < 0 || right < 0) {
    return 0;
  }
  return h->connections[(size_t)left * h->tag_count + (size_t)right] != 0;
}

static void
free_connections(hannanum_t *h)
{
  if (h == NULL) {
    return;
  }
  free(h->connections);
  h->connections = NULL;
}
