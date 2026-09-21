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
text <- intToUtf8(c(0xd55c, 0xae00, 0x20, 0xd615, 0xd0dc, 0xc18c))
MorphAnalyzer(text)
}
