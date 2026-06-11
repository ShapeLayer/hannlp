\name{extractNoun}
\alias{extractNoun}
\title{Noun extractor for Hangul}
\usage{extractNoun(sentences, autoSpacing = FALSE)}
\arguments{
  \item{sentences}{input character vector}
  \item{autoSpacing}{retained for KoNLP API compatibility; currently ignored}
}
\value{character vector for one sentence, list for multiple sentences}
\description{Extract nouns from Korean sentences using the native HanNLP analyzer backend.}
