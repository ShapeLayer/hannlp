\name{SimplePos09}
\alias{SimplePos09}
\title{POS tagging by using 9 KAIST tags}
\usage{SimplePos09(sentences, autoSpacing = FALSE)}
\arguments{
  \item{sentences}{input character vector}
  \item{autoSpacing}{retained for KoNLP API compatibility; currently ignored}
}
\value{named list of eojeol to morpheme/tag strings for one sentence, list for multiple sentences}
\description{Tag Korean sentences using 9 KAIST tags with the native HanNLP analyzer backend.}
