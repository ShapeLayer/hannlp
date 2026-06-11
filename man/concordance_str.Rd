\name{concordance_str}
\alias{concordance_str}
\title{Concordance for input text vector}
\usage{concordance_str(string, pattern, span = 5)}
\arguments{
  \item{string}{input text as character vector or single character}
  \item{pattern}{pattern of central words}
  \item{span}{number of characters to include around each match}
}
\value{list of matched concordance strings, with empty elements removed}
\description{Return concordance text for an input pattern and span.}
