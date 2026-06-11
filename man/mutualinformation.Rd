\name{mutualinformation}
\alias{mutualinformation}
\title{Mutual information for input text}
\usage{mutualinformation(text, query = "", method = c("mutual", "tscores"))}
\arguments{
  \item{text}{input character vector}
  \item{query}{term to filter bigrams}
  \item{method}{calculation method, either \code{mutual} or \code{tscores}}
}
\value{named numeric vector}
\description{Return mutual information or t-scores for adjacent word bigrams.}
