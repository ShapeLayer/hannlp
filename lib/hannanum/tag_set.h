#ifndef HANNANUM_TAGSET_H
#define HANNANUM_TAGSET_H

/* Port of share/TagSet.java tag, tag-group, irregular, and tag-type metadata. */

static int tag_id(hannanum_t *h, const char *tag);
static int irregular_id(hannanum_t *h, const char *name);
static int load_tag_set(hannanum_t *h);
static void free_tag_set(hannanum_t *h);
static int check_tag_type(hannanum_t *h, int type, int tag);
static int check_phoneme_type(int phoneme_type, int phoneme);

#endif
