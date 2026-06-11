#' Concordance for input text vector
#'
#' @param string input text as character vector or single character
#' @param pattern pattern of central words
#' @param span number of characters to include around each match
#' @return list of matched concordance strings, with empty elements removed
#' @export
concordance_str <- function(string, pattern, span = 5) {
  if (!is.character(string) || !is.character(pattern) || length(pattern) != 1L) {
    stop("string and pattern must be character values", call. = FALSE)
  }
  expr <- sprintf(".{0,%d}%s.{0,%d}", as.integer(span), pattern, as.integer(span))
  matches <- gregexpr(expr, string, ignore.case = TRUE, perl = TRUE)
  out <- regmatches(string, matches)
  Filter(function(x) length(x) != 0L && !identical(x, character(0)), out)
}

#' Concordance for input text file
#'
#' @param filename file name
#' @param pattern pattern of central words
#' @param encoding file encoding
#' @param span number of characters to include around each match
#' @return character vector of matched concordance strings
#' @export
concordance_file <- function(filename, pattern, encoding = getOption("encoding"), span = 5) {
  f <- file(filename, "r", encoding = encoding)
  on.exit(close(f), add = TRUE)
  out <- character()
  repeat {
    next_line <- readLines(f, n = 1L, warn = FALSE)
    if (length(next_line) == 0L) {
      break
    }
    ret <- concordance_str(next_line, pattern, span)
    if (length(ret) != 0L) {
      out <- c(out, unlist(ret, use.names = FALSE))
    }
  }
  out
}

.hannlp_ngrams <- function(tokens, n) {
  if (length(tokens) < n) {
    return(character())
  }
  vapply(seq_len(length(tokens) - n + 1L), function(i) {
    paste(tokens[i:(i + n - 1L)], collapse = " ")
  }, character(1L), USE.NAMES = FALSE)
}

.hannlp_tokenize_words <- function(text) {
  lapply(text, function(x) {
    tokens <- unlist(strsplit(x, "[[:space:]]+", perl = TRUE), use.names = FALSE)
    tokens[nzchar(tokens)]
  })
}

#' Mutual information for input text
#'
#' @param text input character vector
#' @param query term to filter bigrams
#' @param method calculation method, either `mutual` or `tscores`
#' @return named numeric vector
#' @export
mutualinformation <- function(text, query = "", method = c("mutual", "tscores")) {
  if (!is.character(text)) {
    stop("text must be a character vector", call. = FALSE)
  }
  method <- match.arg(method)
  token_lines <- .hannlp_tokenize_words(text)
  tokens <- unlist(token_lines, use.names = FALSE)
  if (length(tokens) == 0L) {
    return(numeric())
  }
  bigram_tokens <- unlist(lapply(token_lines, .hannlp_ngrams, n = 2L), use.names = FALSE)
  if (length(bigram_tokens) == 0L) {
    return(numeric())
  }

  unigram <- table(tokens)
  bigram <- table(bigram_tokens)
  num_of_words <- sum(unigram)
  bigram_names <- names(bigram)
  if (!identical(query, "")) {
    bigram_names <- Filter(function(x) query %in% strsplit(x, " ", fixed = TRUE)[[1L]], bigram_names)
  }
  if (length(bigram_names) == 0L) {
    return(numeric())
  }

  stats <- vapply(bigram_names, function(x) {
    bi <- strsplit(x, " ", fixed = TRUE)[[1L]]
    observed <- as.numeric(bigram[[x]])
    expected <- as.numeric(unigram[[bi[1L]]]) * as.numeric(unigram[[bi[2L]]]) / num_of_words
    if (method == "mutual") {
      log(observed / expected)
    } else {
      (observed - expected) / sqrt(observed)
    }
  }, numeric(1L), USE.NAMES = TRUE)
  stats
}
