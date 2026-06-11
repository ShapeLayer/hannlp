#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HANNANUM_HASH_SIZE 262144u
#if defined(__GNUC__) || defined(__clang__)
#define HANNANUM_UNUSED __attribute__((unused))
#else
#define HANNANUM_UNUSED
#endif

typedef struct candidate_list {
  void *items;
  size_t count;
  size_t capacity;
} candidate_list_t;

typedef struct dict_entry {
  char *key;
  candidate_list_t candidates;
  size_t analyzed_count;
  struct dict_entry *next;
} dict_entry_t;

typedef struct prob_entry {
  char *key;
  double value;
  struct prob_entry *next;
} prob_entry_t;

typedef struct {
  char *name;
  int *ids;
  size_t count;
} tag_group_t;

enum {
  HANNANUM_TAG_TYPE_ALL = 0,
  HANNANUM_TAG_TYPE_VERBS = 1,
  HANNANUM_TAG_TYPE_NOUNS = 2,
  HANNANUM_TAG_TYPE_NPS = 3,
  HANNANUM_TAG_TYPE_ADJS = 4,
  HANNANUM_TAG_TYPE_NBNP = 5,
  HANNANUM_TAG_TYPE_JOSA = 6,
  HANNANUM_TAG_TYPE_YONGS = 7,
  HANNANUM_TAG_TYPE_EOMIES = 8,
  HANNANUM_TAG_TYPE_JP = 9,
  HANNANUM_TAG_TYPE_COUNT = 10
};

typedef struct hannanum {
  char *data_dir;
  int output_mode;
  char error[256];
  dict_entry_t **dict;
  prob_entry_t **pwt;
  prob_entry_t **ptt_pos;
  prob_entry_t **ptt_wp;
  char **tag_names;
  size_t tag_count;
  char **irregular_names;
  size_t irregular_count;
  tag_group_t *tag_groups;
  size_t tag_group_count;
  tag_group_t tag_types[HANNANUM_TAG_TYPE_COUNT];
  unsigned char *connections;
} hannanum_t;

#include "strbuf.c"
#include "share.c"
#include "tag_set.c"

int
main(int argc, char **argv)
{
  hannanum_t h;
  int ncn;
  int pvg;
  int irrb;
  if (argc != 2) {
    fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
    return 2;
  }
  memset(&h, 0, sizeof(h));
  h.data_dir = argv[1];
  if (!load_tag_set(&h)) {
    fprintf(stderr, "failed to load tag set\n");
    return 1;
  }
  ncn = tag_id(&h, "ncn");
  pvg = tag_id(&h, "pvg");
  irrb = irregular_id(&h, "irrb");
  if (ncn < 0 || pvg < 0 || irrb < 0) {
    fprintf(stderr, "required tag/irregular missing\n");
    free_tag_set(&h);
    return 1;
  }
  if (!check_tag_type(&h, HANNANUM_TAG_TYPE_NOUNS, ncn)) {
    fprintf(stderr, "ncn not in noun type\n");
    free_tag_set(&h);
    return 1;
  }
  if (!check_tag_type(&h, HANNANUM_TAG_TYPE_VERBS, pvg)) {
    fprintf(stderr, "pvg not in verb type\n");
    free_tag_set(&h);
    return 1;
  }
  if (tag_group_find(&h, "unkset") == NULL) {
    fprintf(stderr, "unkset group missing\n");
    free_tag_set(&h);
    return 1;
  }
  free_tag_set(&h);
  return 0;
}
