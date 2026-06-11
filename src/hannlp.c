#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>

SEXP hannlp_hannanum_analyze(SEXP sentence_sexp, SEXP data_dir_sexp, SEXP mode_sexp);
SEXP hannlp_hangul_is(SEXP input_sexp, SEXP kind_sexp);
SEXP hannlp_hangul_to_jamos(SEXP input_sexp);
SEXP hannlp_hangul_to_keystrokes(SEXP input_sexp, SEXP fullwidth_sexp);
SEXP hannlp_hangul_automata(SEXP input_sexp, SEXP keystroke_sexp, SEXP force_sexp);

static const R_CallMethodDef call_methods[] = {
  {"hannlp_hannanum_analyze", (DL_FUNC) &hannlp_hannanum_analyze, 3},
  {"hannlp_hangul_is", (DL_FUNC) &hannlp_hangul_is, 2},
  {"hannlp_hangul_to_jamos", (DL_FUNC) &hannlp_hangul_to_jamos, 1},
  {"hannlp_hangul_to_keystrokes", (DL_FUNC) &hannlp_hangul_to_keystrokes, 2},
  {"hannlp_hangul_automata", (DL_FUNC) &hannlp_hangul_automata, 3},
  {NULL, NULL, 0}
};

void
R_init_HanNLP(DllInfo *dll)
{
  R_registerRoutines(dll, NULL, call_methods, NULL, NULL);
  R_useDynamicSymbols(dll, FALSE);
}
