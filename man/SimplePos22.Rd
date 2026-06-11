\name{SimplePos22}
\alias{SimplePos22}
\title{POS tagging by using 22 KAIST tags}
\usage{SimplePos22(sentences, autoSpacing = FALSE)}
\arguments{
  \item{sentences}{input character vector}
  \item{autoSpacing}{retained for KoNLP API compatibility; currently ignored}
}
\value{named list of eojeol to morpheme/tag strings for one sentence, list for multiple sentences}
\description{Tag Korean sentences using 22 KAIST tags with the native HanNLP analyzer backend.}
