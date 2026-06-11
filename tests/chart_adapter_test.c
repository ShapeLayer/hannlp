#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hannanum.c"

int
main(int argc, char **argv)
{
  hannanum_options_t options;
  hannanum_t *h;
  int i;
  int dump = 0;
  if (argc < 2) {
    fprintf(stderr, "usage: %s DATA_DIR [WORD...]\n", argv[0]);
    return 2;
  }
  if (argc > 2 && strcmp(argv[2], "--dump") == 0) {
    dump = 1;
    argv++;
    argc--;
  }
  memset(&options, 0, sizeof(options));
  options.data_dir = argv[1];
  h = hannanum_create(&options);
  if (h == NULL) {
    fprintf(stderr, "failed to create hannanum\n");
    return 1;
  }
  if (argc == 2) {
    const char *word = "학교";
    candidate_list_t candidates = chart_candidates_for(h, word);
    if (candidates.count == 0) {
      fprintf(stderr, "chart adapter produced no candidates for %s\n", word);
      hannanum_destroy(h);
      return 1;
    }
    if (dump) {
      size_t j;
      size_t k;
      printf("%s\n", word);
      printf("candidates=%zu\n", candidates.count);
      for (j = 0; j < candidates.count; j++) {
        printf("\t");
        for (k = 0; k < candidates.items[j].length; k++) {
          printf("%s%s/%s", k == 0 ? "" : "+", candidates.items[j].morphemes[k], candidates.items[j].tags[k]);
        }
        printf("\n");
      }
    }
    free_candidate_list(&candidates);
  }
  for (i = 2; i < argc; i++) {
    candidate_list_t candidates = chart_candidates_for(h, argv[i]);
    if (candidates.count == 0) {
      fprintf(stderr, "chart adapter produced no candidates for %s\n", argv[i]);
      hannanum_destroy(h);
      return 1;
    }
    free_candidate_list(&candidates);
  }
  hannanum_destroy(h);
  return 0;
}
