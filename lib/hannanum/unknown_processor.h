#ifndef HANNANUM_UNKNOWN_PROCESSOR_H
#define HANNANUM_UNKNOWN_PROCESSOR_H

/* Port of SupplementPlugin/MorphemeProcessor/UnknownMorphProcessor/UnknownProcessor.java. */

static candidate_list_t candidates_for(hannanum_t *h, const char *plain);
static int unknown_processor_expand_list(candidate_list_t *list);

#endif
