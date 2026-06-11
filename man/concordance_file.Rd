\name{concordance_file}
\alias{concordance_file}
\title{Concordance for input text file}
\usage{concordance_file(filename, pattern, encoding = getOption("encoding"), span = 5)}
\arguments{
  \item{filename}{file name}
  \item{pattern}{pattern of central words}
  \item{encoding}{file encoding}
  \item{span}{number of characters to include around each match}
}
\value{character vector of matched concordance strings}
\description{Return concordance text for an input file.}
