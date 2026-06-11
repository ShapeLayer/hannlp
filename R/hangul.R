.hannlp_check_character <- function(sentence) {
  if (!is.character(sentence) || any(nchar(sentence) == 0L)) {
    stop("Input must be legitimate character!", call. = FALSE)
  }
}

#' Check if sentence is all Hangul
#'
#' @param sentence input characters
#' @return logical vector
#' @export
is.hangul <- function(sentence) {
  .hannlp_check_character(sentence)
  .Call("hannlp_hangul_is", sentence, "hangul", PACKAGE = "HanNLP")
}

#' Check if sentence is all Jamo
#'
#' @param sentence input characters
#' @return logical vector
#' @export
is.jamo <- function(sentence) {
  .hannlp_check_character(sentence)
  .Call("hannlp_hangul_is", sentence, "jamo", PACKAGE = "HanNLP")
}

#' Check if sentence is all Jaeum
#'
#' @param sentence input characters
#' @return logical vector
#' @export
is.jaeum <- function(sentence) {
  .hannlp_check_character(sentence)
  .Call("hannlp_hangul_is", sentence, "jaeum", PACKAGE = "HanNLP")
}

#' Check if sentence is all Moeum
#'
#' @param sentence input characters
#' @return logical vector
#' @export
is.moeum <- function(sentence) {
  .hannlp_check_character(sentence)
  .Call("hannlp_hangul_is", sentence, "moeum", PACKAGE = "HanNLP")
}

#' Check if sentence is all ASCII
#'
#' @param sentence input characters
#' @return logical vector
#' @export
is.ascii <- function(sentence) {
  .hannlp_check_character(sentence)
  .Call("hannlp_hangul_is", sentence, "ascii", PACKAGE = "HanNLP")
}

#' Convert Hangul string to Jamos
#'
#' @param hangul Hangul string
#' @return Jamo sequences
#' @export
convertHangulStringToJamos <- function(hangul) {
  if (!is.character(hangul) || length(hangul) != 1L || nchar(hangul) == 0L) {
    stop("Input must be legitimate character!", call. = FALSE)
  }
  out <- .Call("hannlp_hangul_to_jamos", hangul, PACKAGE = "HanNLP")
  unlist(strsplit(out, intToUtf8(0xFF5C), fixed = TRUE), use.names = FALSE)
}

#' Convert Hangul string to keystrokes
#'
#' @param hangul Hangul sentence
#' @param isFullwidth specify returned character will be Fullwidth ASCII or Halfwidth ASCII
#' @return Keystroke sequence
#' @export
convertHangulStringToKeyStrokes <- function(hangul, isFullwidth = TRUE) {
  if (!is.character(hangul) || length(hangul) != 1L || nchar(hangul) == 0L) {
    stop("Input must be legitimate character!", call. = FALSE)
  }
  out <- .Call("hannlp_hangul_to_keystrokes", hangul, isFullwidth, PACKAGE = "HanNLP")
  unlist(strsplit(out, intToUtf8(0xFF5C), fixed = TRUE), use.names = FALSE)
}

#' Compose Hangul syllables from Jamo or keystrokes
#'
#' @param input input Jamo sequences or two-beolsik keystrokes
#' @param isKeystroke whether input is two-beolsik keystrokes instead of Jamo
#' @param isForceConv force conversion for incomplete sequences
#' @return complete Hangul syllable string
#' @export
HangulAutomata <- function(input, isKeystroke = FALSE, isForceConv = FALSE) {
  if (!is.character(input) || length(input) != 1L || nchar(input) == 0L) {
    stop("Input must be legitimate character!", call. = FALSE)
  }
  .Call("hannlp_hangul_automata", input, isKeystroke, isForceConv, PACKAGE = "HanNLP")
}
