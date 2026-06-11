#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__) || defined(__clang__)
#define HANNANUM_UNUSED __attribute__((unused))
#else
#define HANNANUM_UNUSED
#endif

#include "simti.c"

int
main(void)
{
  simti_t *simti = simti_create();
  unsigned int abc[] = { 'a', 'b', 'c' };
  unsigned int ab[] = { 'a', 'b' };
  unsigned int ax[] = { 'a', 'x' };
  if (simti == NULL) {
    return 1;
  }
  simti_init(simti);
  if (simti_insert(simti, abc, 3, 42) != 1) {
    simti_destroy(simti);
    return 1;
  }
  if (simti_insert(simti, abc, 3, 99) != 0) {
    simti_destroy(simti);
    return 1;
  }
  if (simti_fetch(simti, abc, 3) != 42) {
    simti_destroy(simti);
    return 1;
  }
  if (simti_fetch(simti, ab, 2) != 0) {
    simti_destroy(simti);
    return 1;
  }
  if (simti_search(simti, ax, 2) != 1) {
    simti_destroy(simti);
    return 1;
  }
  simti_destroy(simti);
  return 0;
}
