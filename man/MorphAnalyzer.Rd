\name{MorphAnalyzer}
\alias{MorphAnalyzer}
\title{Hannanum morphological analyzer interface function}
\usage{MorphAnalyzer(sentences, autoSpacing = FALSE)}
\arguments{
  \item{sentences}{input character vector}
  \item{autoSpacing}{retained for KoNLP API compatibility; currently ignored}
}
\value{named list of eojeol to morpheme/tag strings for one sentence, list for multiple sentences}
\description{Analyze Korean sentences using the native HanNLP analyzer backend.}
\examples{
local({
  old <- Sys.getenv("HANNLP_USER_DATA_DIR", unset = NA)
  tmp <- tempfile("hannlp-user-data-")
  on.exit({
    if (is.na(old)) Sys.unsetenv("HANNLP_USER_DATA_DIR") else Sys.setenv(HANNLP_USER_DATA_DIR = old)
    unlink(tmp, recursive = TRUE, force = TRUE)
  })
  Sys.setenv(HANNLP_USER_DATA_DIR = tmp)
  MorphAnalyzer("한글 형태소 분석을 테스트합니다")
})
}
