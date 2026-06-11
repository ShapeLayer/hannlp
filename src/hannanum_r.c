#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

#include <stdlib.h>
#include <string.h>

#include "hannanum.h"

static hannanum_output_mode_t
parse_mode(const char *mode)
{
  if (strcmp(mode, "nouns") == 0) {
    return HANNANUM_OUTPUT_NOUNS;
  }
  if (strcmp(mode, "morph") == 0) {
    return HANNANUM_OUTPUT_MORPH_SIMPLE_22;
  }
  if (strcmp(mode, "simple_pos_09") == 0) {
    return HANNANUM_OUTPUT_SIMPLE_POS_09;
  }
  if (strcmp(mode, "simple_pos_22") == 0) {
    return HANNANUM_OUTPUT_SIMPLE_POS_22;
  }
  error("unsupported HanNanum mode: %s", mode);
  return HANNANUM_OUTPUT_HMM_POS;
}

static SEXP
mk_char(const char *value)
{
  return mkCharCE(value == NULL ? "" : value, CE_UTF8);
}

static SEXP
format_morpheme_tag(const char *morpheme, const char *tag)
{
  size_t len = strlen(morpheme) + strlen(tag) + 2;
  char *buffer = (char *) R_alloc(len, sizeof(char));
  snprintf(buffer, len, "%s/%s", morpheme, tag);
  return mk_char(buffer);
}

static SEXP
result_to_nouns(const hannanum_result_t *result)
{
  size_t i;
  size_t j;
  size_t n = 0;
  SEXP out;

  for (i = 0; i < hannanum_result_eojeol_count(result); i++) {
    n += hannanum_result_morpheme_count(result, i);
  }

  PROTECT(out = allocVector(STRSXP, (R_xlen_t) n));
  n = 0;
  for (i = 0; i < hannanum_result_eojeol_count(result); i++) {
    for (j = 0; j < hannanum_result_morpheme_count(result, i); j++) {
      SET_STRING_ELT(out, (R_xlen_t) n, mk_char(hannanum_result_morpheme(result, i, j)));
      n++;
    }
  }
  UNPROTECT(1);
  return out;
}

static SEXP
result_to_tag_list(const hannanum_result_t *result)
{
  size_t i;
  size_t j;
  size_t n = hannanum_result_eojeol_count(result);
  SEXP out;
  SEXP names;

  PROTECT(out = allocVector(VECSXP, (R_xlen_t) n));
  PROTECT(names = allocVector(STRSXP, (R_xlen_t) n));

  for (i = 0; i < n; i++) {
    size_t m = hannanum_result_morpheme_count(result, i);
    SEXP item;

    PROTECT(item = allocVector(STRSXP, (R_xlen_t) m));
    for (j = 0; j < m; j++) {
      SET_STRING_ELT(
        item,
        (R_xlen_t) j,
        format_morpheme_tag(
          hannanum_result_morpheme(result, i, j),
          hannanum_result_tag(result, i, j)
        )
      );
    }
    SET_VECTOR_ELT(out, (R_xlen_t) i, item);
    UNPROTECT(1);

    SET_STRING_ELT(names, (R_xlen_t) i, mk_char(hannanum_result_plain(result, i)));
  }

  setAttrib(out, R_NamesSymbol, names);
  UNPROTECT(2);
  return out;
}

static SEXP
result_to_candidate_list(const hannanum_result_t *result)
{
  size_t i;
  size_t n = hannanum_result_eojeol_count(result);
  SEXP out;
  SEXP names;

  PROTECT(out = allocVector(VECSXP, (R_xlen_t) n));
  PROTECT(names = allocVector(STRSXP, (R_xlen_t) n));

  for (i = 0; i < n; i++) {
    size_t j;
    size_t c = hannanum_result_candidate_count(result, i);
    SEXP item;

    PROTECT(item = allocVector(STRSXP, (R_xlen_t) c));
    for (j = 0; j < c; j++) {
      size_t k;
      size_t m = hannanum_result_candidate_morpheme_count(result, i, j);
      size_t len = 1;
      char *buffer;
      char *cursor;

      for (k = 0; k < m; k++) {
        const char *morpheme = hannanum_result_candidate_morpheme(result, i, j, k);
        const char *tag = hannanum_result_candidate_tag(result, i, j, k);
        len += strlen(morpheme == NULL ? "" : morpheme) + strlen(tag == NULL ? "" : tag) + 2;
      }

      buffer = (char *) R_alloc(len, sizeof(char));
      cursor = buffer;
      for (k = 0; k < m; k++) {
        const char *morpheme = hannanum_result_candidate_morpheme(result, i, j, k);
        const char *tag = hannanum_result_candidate_tag(result, i, j, k);
        int written;
        if (k != 0) {
          *cursor++ = '+';
        }
        written = snprintf(cursor, len - (size_t) (cursor - buffer), "%s/%s", morpheme == NULL ? "" : morpheme, tag == NULL ? "" : tag);
        if (written > 0) {
          cursor += written;
        }
      }
      *cursor = '\0';
      SET_STRING_ELT(item, (R_xlen_t) j, mk_char(buffer));
    }
    SET_VECTOR_ELT(out, (R_xlen_t) i, item);
    UNPROTECT(1);

    SET_STRING_ELT(names, (R_xlen_t) i, mk_char(hannanum_result_plain(result, i)));
  }

  setAttrib(out, R_NamesSymbol, names);
  UNPROTECT(2);
  return out;
}

SEXP
hannlp_hannanum_analyze(SEXP sentence_sexp, SEXP data_dir_sexp, SEXP mode_sexp)
{
  const char *sentence;
  const char *data_dir;
  const char *mode;
  hannanum_options_t options;
  hannanum_t *hannanum;
  hannanum_result_t *result;
  SEXP out;

  if (!isString(sentence_sexp) || XLENGTH(sentence_sexp) != 1) {
    error("sentence must be a character scalar");
  }
  if (!isString(data_dir_sexp) || XLENGTH(data_dir_sexp) != 1) {
    error("data_dir must be a character scalar");
  }
  if (!isString(mode_sexp) || XLENGTH(mode_sexp) != 1) {
    error("mode must be a character scalar");
  }

  sentence = translateCharUTF8(STRING_ELT(sentence_sexp, 0));
  data_dir = translateCharUTF8(STRING_ELT(data_dir_sexp, 0));
  mode = translateCharUTF8(STRING_ELT(mode_sexp, 0));

  memset(&options, 0, sizeof(options));
  options.data_dir = data_dir;
  options.output_mode = parse_mode(mode);

  hannanum = hannanum_create(&options);
  if (hannanum == NULL) {
    error("failed to initialize HanNanum");
  }

  result = hannanum_analyze(hannanum, sentence);
  if (result == NULL) {
    const char *message = hannanum_error(hannanum);
    hannanum_destroy(hannanum);
    error("HanNanum analysis failed: %s", message);
  }

  if (strcmp(mode, "nouns") == 0) {
    out = PROTECT(result_to_nouns(result));
  } else if (strcmp(mode, "morph") == 0) {
    out = PROTECT(result_to_candidate_list(result));
  } else {
    out = PROTECT(result_to_tag_list(result));
  }

  hannanum_result_destroy(result);
  hannanum_destroy(hannanum);
  UNPROTECT(1);
  return out;
}
