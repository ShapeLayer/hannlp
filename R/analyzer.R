#' Noun extractor for Hangul
#'
#' Extract nouns from Korean sentences using the native HanNLP analyzer backend.
#'
#' @param sentences input character vector
#' @param autoSpacing retained for KoNLP API compatibility; currently ignored
#' @return character vector for one sentence, list for multiple sentences
#' @export
extractNoun <- function(sentences, autoSpacing = FALSE) {
  ress <- lapply(sentences, .hannlp_hannanum_analyze_one, mode = "nouns", autoSpacing = autoSpacing)
  if (length(ress) == 1L) {
    ress[[1L]]
  } else {
    ress
  }
}

#' Hannanum morphological analyzer interface function
#'
#' Analyze Korean sentences using the native HanNLP analyzer backend.
#'
#' @param sentences input character vector
#' @param autoSpacing retained for KoNLP API compatibility; currently ignored
#' @return named list of eojeol to morpheme/tag strings for one sentence, list for multiple sentences
#' @export
MorphAnalyzer <- function(sentences, autoSpacing = FALSE) {
  ress <- lapply(sentences, .hannlp_hannanum_analyze_one, mode = "morph", autoSpacing = autoSpacing)
  if (length(ress) == 1L) {
    ress[[1L]]
  } else {
    ress
  }
}

#' POS tagging by using 22 KAIST tags
#'
#' Tag Korean sentences using 22 KAIST tags with the native HanNLP analyzer backend.
#'
#' @param sentences input character vector
#' @param autoSpacing retained for KoNLP API compatibility; currently ignored
#' @return named list of eojeol to morpheme/tag strings for one sentence, list for multiple sentences
#' @export
SimplePos22 <- function(sentences, autoSpacing = FALSE) {
  ress <- lapply(sentences, .hannlp_hannanum_analyze_one, mode = "simple_pos_22", autoSpacing = autoSpacing)
  if (length(ress) == 1L) {
    ress[[1L]]
  } else {
    ress
  }
}

#' POS tagging by using 9 KAIST tags
#'
#' Tag Korean sentences using 9 KAIST tags with the native HanNLP analyzer backend.
#'
#' @param sentences input character vector
#' @param autoSpacing retained for KoNLP API compatibility; currently ignored
#' @return named list of eojeol to morpheme/tag strings for one sentence, list for multiple sentences
#' @export
SimplePos09 <- function(sentences, autoSpacing = FALSE) {
  ress <- lapply(sentences, .hannlp_hannanum_analyze_one, mode = "simple_pos_09", autoSpacing = autoSpacing)
  if (length(ress) == 1L) {
    ress[[1L]]
  } else {
    ress
  }
}
