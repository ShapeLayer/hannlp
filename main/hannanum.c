#include "hannanum.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strbuf.h"

static char    *
read_stdin(void)
{
  strbuf          buffer;
  strbuf_init(&buffer, 4096);
  for (;;) {
    unsigned char   chunk[2048];
    size_t          n;
    n = fread(chunk, 1, sizeof(chunk), stdin);
    strbuf_put(&buffer, chunk, (bufsize_t)n);
    if (n < 2048) {
      if (ferror(stdin)) {
        strbuf_free(&buffer);
        return NULL;
      }
      break;
    }
  }
  return (char *)strbuf_detach(&buffer);
}

static char    *
join_args(int argc, char **argv, int start)
{
  int             i;
  strbuf          text;
  strbuf_init(&text, 128);
  for (i = start; i < argc; i++) {
    if (i != start) {
      strbuf_putc(&text, ' ');
    }
    strbuf_puts(&text, argv[i]);
  }
  return (char *)strbuf_detach(&text);
}

static void
usage(const char *program)
{
  fprintf(stderr, "usage: %s [--data-dir PATH] [--morph|--simple-ma-09|--simple-ma-22|--simple-pos-09|--simple-pos-22|--nouns] [text...]\n", program);
  fprintf(stderr, "If text is omitted, UTF-8 input is read from stdin.\n");
}

int
main(int argc, char **argv)
{
  hannanum_options_t options;
  hannanum_t     *hannanum;
  hannanum_result_t *result;
  char           *input;
  char           *formatted;
  int             argi = 1;
  memset(&options, 0, sizeof(options));

  while (argi < argc) {
    if (strcmp(argv[argi], "--help") == 0 || strcmp(argv[argi], "-h") == 0) {
      usage(argv[0]);
      return 0;
    }
    if (strcmp(argv[argi], "--data-dir") == 0) {
      if (argi + 1 >= argc) {
        usage(argv[0]);
        return 2;
      }
      options.data_dir = argv[argi + 1];
      argi += 2;
      continue;
    }
    if (strcmp(argv[argi], "--simple-pos-09") == 0) {
      options.output_mode = HANNANUM_OUTPUT_SIMPLE_POS_09;
      argi++;
      continue;
    }
    if (strcmp(argv[argi], "--morph") == 0) {
      options.output_mode = HANNANUM_OUTPUT_MORPH;
      argi++;
      continue;
    }
    if (strcmp(argv[argi], "--simple-ma-09") == 0) {
      options.output_mode = HANNANUM_OUTPUT_MORPH_SIMPLE_09;
      argi++;
      continue;
    }
    if (strcmp(argv[argi], "--simple-ma-22") == 0) {
      options.output_mode = HANNANUM_OUTPUT_MORPH_SIMPLE_22;
      argi++;
      continue;
    }
    if (strcmp(argv[argi], "--simple-pos-22") == 0) {
      options.output_mode = HANNANUM_OUTPUT_SIMPLE_POS_22;
      argi++;
      continue;
    }
    if (strcmp(argv[argi], "--nouns") == 0) {
      options.output_mode = HANNANUM_OUTPUT_NOUNS;
      argi++;
      continue;
    }
    break;
  }

  input = argi < argc ? join_args(argc, argv, argi) : read_stdin();
  if (input == NULL) {
    fprintf(stderr, "failed to read input\n");
    return 1;
  }

  hannanum = hannanum_create(&options);
  if (hannanum == NULL) {
    free(input);
    fprintf(stderr, "failed to initialize hannanum\n");
    return 1;
  }

  result = hannanum_analyze(hannanum, input);
  if (result == NULL) {
    fprintf(stderr, "%s\n", hannanum_error(hannanum));
    hannanum_destroy(hannanum);
    free(input);
    return 1;
  }

  formatted = hannanum_result_format(result);
  if (formatted == NULL) {
    fprintf(stderr, "failed to format result\n");
    hannanum_result_destroy(result);
    hannanum_destroy(hannanum);
    free(input);
    return 1;
  }

  fputs(formatted, stdout);
  free(formatted);
  hannanum_result_destroy(result);
  hannanum_destroy(hannanum);
  free(input);
  return 0;
}
